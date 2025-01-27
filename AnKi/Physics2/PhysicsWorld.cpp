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
	ANKI_ASSERT(ptr);
	PhysicsWorld& world = PhysicsWorld::getSingleton();

	LockGuard lock(world.m_collisionShapes.m_mtx);

	world.m_collisionShapes.m_array.erase(ptr->m_arrayIndex);
}

void PhysicsBodyPtrDeleter::operator()(PhysicsBody* ptr)
{
	ANKI_ASSERT(ptr);
	PhysicsWorld& world = PhysicsWorld::getSingleton();

	world.m_jphPhysicsSystem->GetBodyInterface().RemoveBody(ptr->m_jphBody->GetID());
	world.m_jphPhysicsSystem->GetBodyInterface().DestroyBody(ptr->m_jphBody->GetID());

	LockGuard lock(world.m_bodies.m_mtx);
	world.m_bodies.m_array.erase(ptr->m_arrayIndex);
	world.m_optimizeBroadphase = true;
}

void PhysicsJointPtrDeleter::operator()(PhysicsJoint* ptr)
{
	ANKI_ASSERT(ptr);
	PhysicsWorld& world = PhysicsWorld::getSingleton();

	LockGuard lock(world.m_joints.m_mtx);
	world.m_joints.m_array.erase(ptr->m_arrayIndex);
}

void PhysicsPlayerControllerPtrDeleter::operator()(PhysicsPlayerController* ptr)
{
	ANKI_ASSERT(ptr);
	PhysicsWorld& world = PhysicsWorld::getSingleton();

	LockGuard lock(world.m_characters.m_mtx);
	world.m_characters.m_array.erase(ptr->m_arrayIndex);
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
	ANKI_ASSERT(m_collisionShapes.m_array.getSize() == 0);
	ANKI_ASSERT(m_bodies.m_array.getSize() == 0);
	ANKI_ASSERT(m_joints.m_array.getSize() == 0);
	ANKI_ASSERT(m_characters.m_array.getSize() == 0);

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
	// Pre-update work
	{
		for(PhysicsPlayerController& charController : m_characters.m_array)
		{
			charController.prePhysicsUpdate(dt);
		}

		if(m_optimizeBroadphase)
		{
			m_optimizeBroadphase = false;
			m_jphPhysicsSystem->OptimizeBroadPhase();
		}
	}

	constexpr I32 collisionSteps = 1;
	m_jphPhysicsSystem->Update(F32(dt), collisionSteps, &m_tempAllocator, &m_jobSystem);

	// Post-update work
	{
		for(PhysicsBody& body : m_bodies.m_array)
		{
			body.postPhysicsUpdate();
		}

		for(PhysicsPlayerController& charController : m_characters.m_array)
		{
			charController.postPhysicsUpdate();
		}
	}
}

template<typename TJPHCollisionShape, typename... TArgs>
PhysicsCollisionShapePtr PhysicsWorld::newCollisionShape(PhysicsCollisionShape::ShapeType type, TArgs&&... args)
{
	decltype(m_collisionShapes.m_array)::Iterator it;

	{
		LockGuard lock(m_collisionShapes.m_mtx);
		it = m_collisionShapes.m_array.emplace(type);
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

PhysicsCollisionShapePtr PhysicsWorld::newCapsuleCollisionShape(F32 height, F32 radius)
{
	return newCollisionShape<JPH::CapsuleShape>(PhysicsCollisionShape::ShapeType::kCapsule, height / 2.0f, radius);
}

PhysicsBodyPtr PhysicsWorld::newPhysicsBody(const PhysicsBodyInitInfo& init)
{
	PhysicsBody* newBody;
	{
		LockGuard lock(m_bodies.m_mtx);

		m_optimizeBroadphase = true;
		auto it = m_bodies.m_array.emplace();

		newBody = &(*it);
		newBody->m_arrayIndex = it.getArrayIndex();
	}

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
	settings.mUserData = ptrToNumber(newBody);

	// Call the thread-safe version because characters will be creating bodies as well
	JPH::Body* jphBody = m_jphPhysicsSystem->GetBodyInterface().CreateBody(settings);
	m_jphPhysicsSystem->GetBodyInterface().AddBody(jphBody->GetID(), JPH::EActivation::Activate);

	// Create AnKi body
	newBody->m_jphBody = jphBody;
	newBody->m_primaryShape.reset(init.m_shape);
	newBody->m_scaledShape = scaledShape;
	newBody->m_worldTrf = init.m_transform;

	return PhysicsBodyPtr(newBody);
}

template<typename TJPHJoint, typename... TArgs>
PhysicsJointPtr PhysicsWorld::newJoint(PhysicsJoint::Type type, PhysicsBody* body1, PhysicsBody* body2, TArgs&&... args)
{
	ANKI_ASSERT(body1 && body2);

	decltype(m_joints.m_array)::Iterator it;
	{
		LockGuard lock(m_joints.m_mtx);
		it = m_joints.m_array.emplace(type);
	}

	ClassWrapper<TJPHJoint>& classw = reinterpret_cast<ClassWrapper<TJPHJoint>&>(it->m_base);
	classw.construct(*body1->m_jphBody, *body2->m_jphBody, std::forward<TArgs>(args)...);
	classw->SetEmbedded();

	it->m_body1.reset(body1);
	it->m_body2.reset(body2);
	it->m_arrayIndex = it.getArrayIndex();

	m_jphPhysicsSystem->AddConstraint(&it->m_base); // It's thread-safe

	return PhysicsJointPtr(&(*it));
}

PhysicsJointPtr PhysicsWorld::newPointJoint(PhysicsBody* body1, PhysicsBody* body2, Bool pointsInWorldSpace, const Vec3& body1Point,
											const Vec3& body2Point)
{
	JPH::PointConstraintSettings settings;
	settings.SetEmbedded();

	settings.mSpace = (pointsInWorldSpace) ? JPH::EConstraintSpace::WorldSpace : JPH::EConstraintSpace::LocalToBodyCOM;
	settings.mPoint1 = toJPH(body1Point);
	settings.mPoint2 = toJPH(body2Point);

	return newJoint<JPH::PointConstraint>(PhysicsJoint::Type::kPoint, body1, body2, settings);
}

PhysicsPlayerControllerPtr PhysicsWorld::newPlayerController(const PhysicsPlayerControllerInitInfo& init)
{
	PhysicsPlayerController* newChar;
	{
		LockGuard lock(m_characters.m_mtx);

		auto it = m_characters.m_array.emplace();
		newChar = &(*it);
		newChar->m_arrayIndex = it.getArrayIndex();
	}

	newChar->init(init);
	return PhysicsPlayerControllerPtr(newChar);
}

} // namespace v2
} // end namespace anki
