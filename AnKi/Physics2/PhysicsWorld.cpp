// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Physics2/PhysicsWorld.h>
#include <AnKi/Util/System.h>

namespace anki {
namespace v2 {

class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
	static constexpr JPH::BroadPhaseLayer kStaticBroadPhaseLayer{0};
	static constexpr JPH::BroadPhaseLayer kDynamicBroadPhaseLayer{1};

	virtual U32 GetNumBroadPhaseLayers() const override
	{
		return 2;
	}

	virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
	{
		switch(PhysicsLayer(inLayer))
		{
		case PhysicsLayer::kStatic:
			return kStaticBroadPhaseLayer;
		default:
			return kDynamicBroadPhaseLayer;
		}
	}
};

// Class that determines if an object layer can collide with a broadphase layer
class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
	virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
	{
		switch(PhysicsLayer(inLayer1))
		{
		case PhysicsLayer::kStatic:
			return inLayer2 != BPLayerInterfaceImpl::kStaticBroadPhaseLayer; // Static doesn't collide with static.
		default:
			return true;
		}
	}
};

/// Class that determines if two object layers can collide
class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
{
public:
	virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
	{
		switch(PhysicsLayer(inObject1))
		{
		case PhysicsLayer::kStatic:
			return PhysicsLayer(inObject2) != PhysicsLayer::kStatic; // Static doesn't collide with static.
		default:
			return true; // Moving collides with everything
		}
	}
};

static BPLayerInterfaceImpl g_bPLayerInterfaceImpl;
static ObjectVsBroadPhaseLayerFilterImpl g_objectVsBroadPhaseLayerFilterImpl;
static ObjectLayerPairFilterImpl g_objectLayerPairFilterImpl;

void PhysicsCollisionShapePtrDeleter::operator()(PhysicsCollisionShape* ptr)
{
	PhysicsWorld& world = PhysicsWorld::getSingleton();

	LockGuard lock(world.m_mtx);

	world.m_collisionShapes.erase(ptr->m_arrayIndex);
}

void PhysicsBodyPtrDeleter::operator()(PhysicsBody* ptr)
{
	PhysicsWorld& world = PhysicsWorld::getSingleton();

	LockGuard lock(world.m_mtx);

	if(!ptr->m_jphBody->IsInBroadPhase())
	{
		// Hasn't been added yet to the physics system, do special cleanup

		decltype(world.m_bodiesToAdd)::Iterator it = world.m_bodiesToAdd.getBegin();
		while(it != world.m_bodiesToAdd.getEnd())
		{
			if(*it == ptr)
			{
				break;
			}
		}

		ANKI_ASSERT(it != world.m_bodiesToAdd.getEnd() && "Should have been in the \"toAdd\" list");
		world.m_bodiesToAdd.erase(it);

		world.m_bodies.erase(ptr->m_arrayIndex);
	}
	else
	{
		// Deferred cleanup

		world.m_bodiesToRemove.emplaceBack(ptr);
	}
}

void PhysicsWorld::MyBodyActivationListener::OnBodyActivated([[maybe_unused]] const JPH::BodyID& inBodyID, U64 inBodyUserData)
{
	PhysicsBody* body = numberToPtr<PhysicsBody*>(inBodyUserData);
	body->m_activated = 1;
}

void PhysicsWorld::MyBodyActivationListener::OnBodyDeactivated([[maybe_unused]] const JPH::BodyID& inBodyID, U64 inBodyUserData)
{
	PhysicsBody* body = numberToPtr<PhysicsBody*>(inBodyUserData);
	body->m_activated = 0;
}

PhysicsWorld::~PhysicsWorld()
{
	removeBodies();

	ANKI_ASSERT(m_collisionShapes.getSize() == 0);
	ANKI_ASSERT(m_bodies.getSize() == 0);

	m_jobSystem.destroy();
	m_jphPhysicsSystem.destroy();

	JPH::UnregisterTypes();

	delete JPH::Factory::sInstance;
	JPH::Factory::sInstance = nullptr;

	PhysicsMemoryPool::freeSingleton();
}

Error PhysicsWorld::init(AllocAlignedCallback allocCb, void* allocCbData)
{
	PhysicsMemoryPool::allocateSingleton(allocCb, allocCbData);

	JPH::Allocate = [](PtrSize size) -> void* {
		return PhysicsMemoryPool::getSingleton().allocate(size, 16);
	};

	JPH::Reallocate = []([[maybe_unused]] void* in, [[maybe_unused]] PtrSize oldSize, PtrSize newSize) -> void* {
		void* out;

		if(newSize > 0)
		{
			out = PhysicsMemoryPool::getSingleton().allocate(newSize, 16);
		}
		else
		{
			out = nullptr;
		}

		if(in && out)
		{
			ANKI_ASSERT(oldSize > 0 && newSize > 0);
			memcpy(out, in, min(oldSize, newSize));
		}

		PhysicsMemoryPool::getSingleton().free(in);

		return out;
	};

	JPH::Free = [](void* ptr) {
		PhysicsMemoryPool::getSingleton().free(ptr);
	};

	JPH::AlignedAllocate = [](PtrSize size, PtrSize alignment) -> void* {
		return PhysicsMemoryPool::getSingleton().allocate(size, alignment);
	};

	JPH::AlignedFree = [](void* ptr) {
		PhysicsMemoryPool::getSingleton().free(ptr);
	};

	JPH::Factory::sInstance = new JPH::Factory();

	JPH::RegisterTypes();

	m_jphPhysicsSystem.construct();

	constexpr U32 maxBodies = kMaxU16;
	constexpr U32 mutexCount = 0;
	constexpr U32 maxBodyPairs = kMaxU16;
	constexpr U32 maxConstraints = 10 * 1024;
	m_jphPhysicsSystem->Init(maxBodies, mutexCount, maxBodyPairs, maxConstraints, g_bPLayerInterfaceImpl, g_objectVsBroadPhaseLayerFilterImpl,
							 g_objectLayerPairFilterImpl);

	m_jphPhysicsSystem->SetGravity(toJPH(Vec3(0.0f, -9.81f, 0.0f)));

	const U32 threadCount = min(8u, getCpuCoresCount() - 1);
	m_jobSystem.construct(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, threadCount);

	m_tempAllocator.construct(U32(10_MB));

	return Error::kNone;
}

void PhysicsWorld::update(Second dt)
{
	const Bool optimizeBroadphase = m_bodiesToAdd.getSize() || m_bodiesToRemove.getSize();

	removeBodies();
	addBodies();

	if(optimizeBroadphase)
	{
		m_jphPhysicsSystem->OptimizeBroadPhase();
	}

	constexpr I32 collisionSteps = 1;
	m_jphPhysicsSystem->Update(F32(dt), collisionSteps, &m_tempAllocator, &m_jobSystem);

	// Update transforms
	for(auto& body : m_bodies)
	{
		if(body.m_activated)
		{
			const Transform newTrf = toAnKi(body.m_jphBody->GetWorldTransform());
			if(newTrf != body.m_worldTrf)
			{
				body.m_worldTrf = newTrf;
				++body.m_worldTrfVersion;
			}
		}
	}
}

template<typename TJPHCollisionShape, typename... TArgs>
PhysicsCollisionShapePtr PhysicsWorld::newCollisionShape(PhysicsCollisionShape::ShapeType type, TArgs&&... args)
{
	decltype(m_collisionShapes)::Iterator it;

	{
		LockGuard lock(m_mtx);
		it = m_collisionShapes.emplace(type);
	}

	ClassWrapper<TJPHCollisionShape>& classw = reinterpret_cast<ClassWrapper<TJPHCollisionShape>&>(it->m_shapeBase);
	classw.construct(std::forward<TArgs>(args)...);
	classw->SetEmbedded();

	it->m_arrayIndex = it.getArrayIndex();
	return PhysicsCollisionShapePtr(&(*it));
}

PhysicsCollisionShapePtr PhysicsWorld::newSphereCollisionShape(F32 radius)
{
	return newCollisionShape<JPH::SphereShape>(PhysicsCollisionShape::ShapeType::kSphere, radius);
}

PhysicsCollisionShapePtr PhysicsWorld::newBoxCollisionShape(Vec3 extend)
{
	return newCollisionShape<JPH::BoxShape>(PhysicsCollisionShape::ShapeType::kBox, toJPH(extend));
}

PhysicsBodyPtr PhysicsWorld::newPhysicsBody(const PhysicsBodyInitInfo& init)
{
	const Vec3 pos = init.m_transform.getOrigin().xyz();
	const Quat rot = Quat(init.m_transform.getRotation());

	// Create a scale shape
	const Bool hasScale = (init.m_transform.getScale().xyz() - 1.0).getLengthSquared() > kEpsilonf * 10.0;
	PhysicsCollisionShapePtr scaledShape;
	if(hasScale)
	{
		scaledShape = newCollisionShape<JPH::ScaledShape>(PhysicsCollisionShape::ShapeType::kScaled, &init.m_shape->m_shapeBase,
														  toJPH(init.m_transform.getScale().xyz()));
	}

	// Create JPH body
	JPH::BodyCreationSettings settings((scaledShape) ? &scaledShape->m_scaled : &init.m_shape->m_shapeBase, toJPH(pos), toJPH(rot),
									   (init.m_mass == 0.0f) ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic, JPH::ObjectLayer(init.m_layer));

	if(init.m_mass)
	{
		settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
		settings.mMassPropertiesOverride.mMass = init.m_mass;
	}

	settings.mFriction = init.m_friction;

	JPH::Body* jphBody = m_jphPhysicsSystem->GetBodyInterface().CreateBody(settings);

	// Create AnKi body
	LockGuard lock(m_mtx);

	auto it = m_bodies.emplace();
	it->m_jphBody = jphBody;
	it->m_arrayIndex = it.getArrayIndex();
	it->m_primaryShape.reset(init.m_shape);
	it->m_scaledShape = scaledShape;
	it->m_worldTrf = init.m_transform;

	jphBody->SetUserData(ptrToNumber(&(*it)));

	m_bodiesToAdd.emplaceBack(&(*it));

	return PhysicsBodyPtr(&(*it));
}

void PhysicsWorld::addBodies()
{
	if(m_bodiesToAdd.getSize() == 0) [[likely]]
	{
		return;
	}

	PhysicsDynamicArray<JPH::BodyID> jphBodies;
	jphBodies.resize(m_bodiesToAdd.getSize());

	for(U32 i = 0; i < m_bodiesToAdd.getSize(); ++i)
	{
		jphBodies[i] = m_bodiesToAdd[i]->m_jphBody->GetID();
	}

	const JPH::BodyInterface::AddState state = m_jphPhysicsSystem->GetBodyInterface().AddBodiesPrepare(jphBodies.getBegin(), jphBodies.getSize());
	m_jphPhysicsSystem->GetBodyInterface().AddBodiesFinalize(jphBodies.getBegin(), jphBodies.getSize(), state, JPH::EActivation::Activate);

	for(U32 i = 0; i < m_bodiesToAdd.getSize(); ++i)
	{
		ANKI_ASSERT(m_bodiesToAdd[i]->m_jphBody->IsInBroadPhase());
	}

	m_bodiesToAdd.resize(0);
}

void PhysicsWorld::removeBodies()
{
	if(m_bodiesToRemove.getSize() == 0) [[likely]]
	{
		return;
	}

	// 1st, remove from phys system
	PhysicsDynamicArray<JPH::BodyID> jphBodies;
	jphBodies.resize(m_bodiesToRemove.getSize());
	for(U32 i = 0; i < m_bodiesToRemove.getSize(); ++i)
	{
		jphBodies[i] = m_bodiesToRemove[i]->m_jphBody->GetID();
	}
	m_jphPhysicsSystem->GetBodyInterface().RemoveBodies(jphBodies.getBegin(), jphBodies.getSize());

	// 2nd, delete the JPH bodies
	m_jphPhysicsSystem->GetBodyInterface().DestroyBodies(jphBodies.getBegin(), jphBodies.getSize());

	// 3rd, delete the AnKi bodies
	for(U32 i = 0; i < m_bodiesToRemove.getSize(); ++i)
	{
		m_bodies.erase(m_bodiesToRemove[i]->m_arrayIndex);
	}

	m_bodiesToRemove.resize(0);
}

} // namespace v2
} // end namespace anki
