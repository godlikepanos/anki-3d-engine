// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/physics/PhysicsWorld.h>
#include <anki/physics/PhysicsCollisionShape.h>
#include <anki/physics/PhysicsBody.h>
#include <anki/physics/PhysicsTrigger.h>
#include <anki/util/Rtti.h>
#include <BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>

namespace anki
{

// Ugly but there is no other way
static HeapAllocator<U8>* gAlloc = nullptr;

static void* btAlloc(size_t size)
{
	ANKI_ASSERT(gAlloc);
	return gAlloc->getMemoryPool().allocate(size, 16);
}

static void btFree(void* ptr)
{
	ANKI_ASSERT(gAlloc);
	gAlloc->getMemoryPool().free(ptr);
}

/// Broad phase collision callback.
class PhysicsWorld::MyOverlapFilterCallback : public btOverlapFilterCallback
{
public:
	bool needBroadphaseCollision(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) const override
	{
		ANKI_ASSERT(proxy0 && proxy1);

		const btCollisionObject* btObj0 = static_cast<const btCollisionObject*>(proxy0->m_clientObject);
		const btCollisionObject* btObj1 = static_cast<const btCollisionObject*>(proxy1->m_clientObject);
		ANKI_ASSERT(btObj0 && btObj1);

		const PhysicsObject* aobj0 = static_cast<const PhysicsObject*>(btObj0->getUserPointer());
		const PhysicsObject* aobj1 = static_cast<const PhysicsObject*>(btObj1->getUserPointer());

		if(aobj0 == nullptr || aobj1 == nullptr)
		{
			return false;
		}

		const PhysicsFilteredObject* fobj0 = dcast<const PhysicsFilteredObject*>(aobj0);
		const PhysicsFilteredObject* fobj1 = dcast<const PhysicsFilteredObject*>(aobj1);

		// First check the masks
		Bool collide = !!(fobj0->getMaterialGroup() & fobj1->getMaterialMask());
		collide = collide && !!(fobj1->getMaterialGroup() & fobj0->getMaterialMask());
		if(!collide)
		{
			return false;
		}

		// Reject if they are both static
		if(ANKI_UNLIKELY(fobj0->getMaterialGroup() == PhysicsMaterialBit::STATIC_GEOMETRY
						 && fobj1->getMaterialGroup() == PhysicsMaterialBit::STATIC_GEOMETRY))
		{
			return false;
		}

		// Detailed tests using callbacks
		if(fobj0->getPhysicsBroadPhaseFilterCallback())
		{
			collide = fobj0->getPhysicsBroadPhaseFilterCallback()->needsCollision(*fobj0, *fobj1);
			if(!collide)
			{
				return false;
			}
		}

		if(fobj1->getPhysicsBroadPhaseFilterCallback())
		{
			collide = fobj1->getPhysicsBroadPhaseFilterCallback()->needsCollision(*fobj1, *fobj0);
			if(!collide)
			{
				return false;
			}
		}

		return true;
	}
};

class PhysicsWorld::MyRaycastCallback : public btCollisionWorld::RayResultCallback
{
public:
	PhysicsWorldRayCastCallback* m_raycast = nullptr;

	bool needsCollision(btBroadphaseProxy* proxy) const override
	{
		ANKI_ASSERT(proxy);

		const btCollisionObject* cobj = static_cast<const btCollisionObject*>(proxy->m_clientObject);
		ANKI_ASSERT(cobj);

		const PhysicsObject* pobj = static_cast<const PhysicsObject*>(cobj->getUserPointer());
		ANKI_ASSERT(pobj);

		const PhysicsFilteredObject* fobj = dcast<const PhysicsFilteredObject*>(pobj);

		return !!(fobj->getMaterialGroup() & m_raycast->m_materialMask);
	}

	btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace) final
	{
		// No idea why
		if(m_raycast->m_firstHit)
		{
			m_closestHitFraction = rayResult.m_hitFraction;
		}

		m_collisionObject = rayResult.m_collisionObject;
		Vec3 worldNormal;
		if(normalInWorldSpace)
		{
			worldNormal = toAnki(rayResult.m_hitNormalLocal);
		}
		else
		{
			worldNormal = toAnki(m_collisionObject->getWorldTransform().getBasis() * rayResult.m_hitNormalLocal);
		}

		Vec3 hitPointWorld = mix(m_raycast->m_from, m_raycast->m_to, rayResult.m_hitFraction);

		// Call the callback
		PhysicsObject* pobj = static_cast<PhysicsObject*>(m_collisionObject->getUserPointer());
		ANKI_ASSERT(pobj);
		m_raycast->processResult(dcast<PhysicsFilteredObject&>(*pobj), worldNormal, hitPointWorld);

		return m_closestHitFraction;
	}
};

PhysicsWorld::PhysicsWorld()
{
}

PhysicsWorld::~PhysicsWorld()
{
#if ANKI_ASSERTS_ENABLED
	for(PhysicsObjectType type = PhysicsObjectType::FIRST; type < PhysicsObjectType::COUNT; ++type)
	{
		ANKI_ASSERT(m_objectLists[type].isEmpty() && "Someone is holding refs to some physics objects");
	}
#endif

	m_world.destroy();
	m_solver.destroy();
	m_dispatcher.destroy();
	m_collisionConfig.destroy();
	m_broadphase.destroy();
	m_gpc.destroy();
	m_alloc.deleteInstance(m_filterCallback);

	gAlloc = nullptr;
}

Error PhysicsWorld::create(AllocAlignedCallback allocCb, void* allocCbData)
{
	m_alloc = HeapAllocator<U8>(allocCb, allocCbData);
	m_tmpAlloc = StackAllocator<U8>(allocCb, allocCbData, 1_KB, 2.0f);

	// Set allocators
	gAlloc = &m_alloc;
	btAlignedAllocSetCustom(btAlloc, btFree);

	// Create objects
	m_broadphase.init();
	m_gpc.init();
	m_broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(m_gpc.get());
	m_filterCallback = m_alloc.newInstance<MyOverlapFilterCallback>();
	m_broadphase->getOverlappingPairCache()->setOverlapFilterCallback(m_filterCallback);

	m_collisionConfig.init();

	m_dispatcher.init(m_collisionConfig.get());
	btGImpactCollisionAlgorithm::registerAlgorithm(m_dispatcher.get());

	m_solver.init();

	m_world.init(m_dispatcher.get(), m_broadphase.get(), m_solver.get(), m_collisionConfig.get());
	m_world->setGravity(btVector3(0.0f, -9.8f, 0.0f));

	return Error::NONE;
}

Error PhysicsWorld::update(Second dt)
{
	// Update world
	{
		auto lock = lockBtWorld();
		m_world->stepSimulation(dt, 1, 1.0 / 60.0);
	}

	// Process trigger contacts
	{
		LockGuard<Mutex> lock(m_objectListsMtx);

		for(PhysicsObject& trigger : m_objectLists[PhysicsObjectType::TRIGGER])
		{
			static_cast<PhysicsTrigger&>(trigger).processContacts();
		}
	}

	// Reset the pool
	m_tmpAlloc.getMemoryPool().reset();

	return Error::NONE;
}

void PhysicsWorld::destroyObject(PhysicsObject* obj)
{
	ANKI_ASSERT(obj);

	{
		LockGuard<Mutex> lock(m_objectListsMtx);
		m_objectLists[obj->getType()].erase(obj);
	}

	obj->~PhysicsObject();
	m_alloc.getMemoryPool().free(obj);
}

void PhysicsWorld::rayCast(WeakArray<PhysicsWorldRayCastCallback*> rayCasts)
{
	auto lock = lockBtWorld();

	MyRaycastCallback callback;
	for(PhysicsWorldRayCastCallback* cb : rayCasts)
	{
		callback.m_raycast = cb;
		m_world->rayTest(toBt(cb->m_from), toBt(cb->m_to), callback);
	}
}

} // end namespace anki
