// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Physics2/PhysicsWorld.h>
#include <AnKi/Util/System.h>

namespace anki {
namespace v2 {

class BodyOrController
{
public:
	union
	{
		PhysicsBody* m_body = nullptr;
		PhysicsPlayerController* m_controller;
	};

	Bool m_isBody = false;
};

static BodyOrController decodeBodyUserData(U64 userData)
{
	BodyOrController out;
	ANKI_ASSERT(userData);
	if(userData & 1u)
	{
		out.m_controller = numberToPtr<PhysicsPlayerController*>(userData & (~1_U64));
		out.m_isBody = false;
	}
	else
	{
		out.m_body = numberToPtr<PhysicsBody*>(userData);
		out.m_isBody = true;
	}

	return out;
}

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

class PhysicsWorld::MyBodyActivationListener final : public JPH::BodyActivationListener
{
public:
	void OnBodyActivated([[maybe_unused]] const JPH::BodyID& inBodyID, U64 bodyUserData) override
	{
		const BodyOrController userData = decodeBodyUserData(bodyUserData);

		if(userData.m_isBody)
		{
			userData.m_body->m_activated = 1;
		}
		else
		{
			// It's a player controller, do nothing for now
		}
	}

	void OnBodyDeactivated([[maybe_unused]] const JPH::BodyID& inBodyID, U64 bodyUserData) override
	{
		const BodyOrController userData = decodeBodyUserData(bodyUserData);

		if(userData.m_isBody)
		{
			userData.m_body->m_activated = 0;
		}
		else
		{
			// It's a player controller, do nothing for now
		}
	}
};

class PhysicsWorld::MyContactListener final : public JPH::ContactListener
{
public:
	void gatherObjects(U64 body1UserData, U64 body2UserData, PhysicsBody*& trigger, BodyOrController& receiver)
	{
		const BodyOrController userData1 = decodeBodyUserData(body1UserData);
		const BodyOrController userData2 = decodeBodyUserData(body2UserData);

		if(userData1.m_isBody && userData1.m_body->m_isTrigger)
		{
			trigger = userData1.m_body;
			receiver = userData2;
		}
		else if(userData2.m_isBody && userData2.m_body->m_isTrigger)
		{
			trigger = userData2.m_body;
			receiver = userData1;
		}
		else
		{
			trigger = nullptr;
			receiver = {};
		}
	}

	void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, [[maybe_unused]] const JPH::ContactManifold& inManifold,
						[[maybe_unused]] JPH::ContactSettings& ioSettings) override
	{
		// You can practically do nothing with the bodies so stash them

		PhysicsBody* trigger;
		BodyOrController receiver;
		gatherObjects(inBody1.GetUserData(), inBody2.GetUserData(), trigger, receiver);

		if(!trigger || trigger->m_triggerCallbacks == nullptr)
		{
			return;
		}

		const Contact contact = {trigger, receiver.m_body, receiver.m_isBody};

		PhysicsWorld& world = PhysicsWorld::getSingleton();

		LockGuard lock(world.m_insertedContactsMtx);
		world.m_insertedContacts.emplaceBack(contact);
	}

	void OnContactRemoved(const JPH::SubShapeIDPair& pair) override
	{
		// You can practically do nothing with the bodies so stash them

		PhysicsWorld& world = PhysicsWorld::getSingleton();

		PhysicsBody* trigger;
		BodyOrController receiver;
		gatherObjects(world.m_jphPhysicsSystem->GetBodyInterfaceNoLock().GetUserData(pair.GetBody1ID()),
					  world.m_jphPhysicsSystem->GetBodyInterfaceNoLock().GetUserData(pair.GetBody2ID()), trigger, receiver);

		if(!trigger || trigger->m_triggerCallbacks == nullptr)
		{
			return;
		}

		const Contact contact = {trigger, receiver.m_body, receiver.m_isBody};

		LockGuard lock(world.m_deletedContactsMtx);
		world.m_deletedContacts.emplaceBack(contact);
	}
};

// Globals
static BPLayerInterfaceImpl g_bPLayerInterfaceImpl;
static ObjectVsBroadPhaseLayerFilterImpl g_objectVsBroadPhaseLayerFilterImpl;
static ObjectLayerPairFilterImpl g_objectLayerPairFilterImpl;
PhysicsWorld::MyBodyActivationListener PhysicsWorld::m_bodyActivationListener;
PhysicsWorld::MyContactListener PhysicsWorld::m_contactListener;

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
	m_jphPhysicsSystem->SetBodyActivationListener(&m_bodyActivationListener);
	m_jphPhysicsSystem->SetContactListener(&m_contactListener);

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

		for(Contact& contact : m_insertedContacts)
		{
			if(contact.m_recieverIsBody)
			{
				contact.m_trigger->m_triggerCallbacks->onTriggerEnter(*contact.m_trigger, *contact.m_recieverBody);
			}
			else
			{
				contact.m_trigger->m_triggerCallbacks->onTriggerEnter(*contact.m_trigger, *contact.m_recieverController);
			}
		}

		m_insertedContacts.destroy();

		for(Contact& contact : m_deletedContacts)
		{
			if(contact.m_recieverIsBody)
			{
				contact.m_trigger->m_triggerCallbacks->onTriggerExit(*contact.m_trigger, *contact.m_recieverBody);
			}
			else
			{
				contact.m_trigger->m_triggerCallbacks->onTriggerExit(*contact.m_trigger, *contact.m_recieverController);
			}
		}

		m_deletedContacts.destroy();
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

PhysicsCollisionShapePtr PhysicsWorld::newScaleCollisionObject(const Vec3& scale, PhysicsCollisionShape* baseShape)
{
	return newCollisionShape<JPH::ScaledShape>(PhysicsCollisionShape::ShapeType::kScaled, &baseShape->m_shapeBase, toJPH(scale));
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

	newBody->init(init);
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
