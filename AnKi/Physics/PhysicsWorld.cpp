// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Physics/PhysicsWorld.h>
#include <AnKi/Physics/PhysicsCollisionShape.h>
#include <AnKi/Physics/PhysicsBody.h>
#include <AnKi/Physics/PhysicsTrigger.h>
#include <AnKi/Physics/PhysicsPlayerController.h>
#include <AnKi/Util/Rtti.h>
#include <BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>

namespace anki {

// Ugly but there is no other way
static HeapMemoryPool* g_pool = nullptr;

static void* btAlloc(size_t size)
{
	ANKI_ASSERT(g_pool);
	return g_pool->allocate(size, 16);
}

static void btFree(void* ptr)
{
	ANKI_ASSERT(g_pool);
	g_pool->free(ptr);
}

/// Broad phase collision callback.
class PhysicsWorld::MyOverlapFilterCallback : public btOverlapFilterCallback
{
public:
	Bool needBroadphaseCollision(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) const override
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
		if(ANKI_UNLIKELY(fobj0->getMaterialGroup() == PhysicsMaterialBit::kStaticGeometry
						 && fobj1->getMaterialGroup() == PhysicsMaterialBit::kStaticGeometry))
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

	Bool needsCollision(btBroadphaseProxy* proxy) const override
	{
		ANKI_ASSERT(proxy);

		const btCollisionObject* cobj = static_cast<const btCollisionObject*>(proxy->m_clientObject);
		ANKI_ASSERT(cobj);

		const PhysicsObject* pobj = static_cast<const PhysicsObject*>(cobj->getUserPointer());
		ANKI_ASSERT(pobj);

		const PhysicsFilteredObject* fobj = dcast<const PhysicsFilteredObject*>(pobj);

		return !!(fobj->getMaterialGroup() & m_raycast->m_materialMask);
	}

	btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, Bool normalInWorldSpace) final
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
	destroyMarkedForDeletion();

	ANKI_ASSERT(m_objectsCreatedCount.load() == 0 && "Forgot to delete some objects");

	m_world.destroy();
	m_solver.destroy();
	m_dispatcher.destroy();
	m_collisionConfig.destroy();
	m_broadphase.destroy();
	m_gpc.destroy();
	deleteInstance(m_pool, m_filterCallback);

	g_pool = nullptr;
}

Error PhysicsWorld::init(AllocAlignedCallback allocCb, void* allocCbData)
{
	m_pool.init(allocCb, allocCbData);
	m_tmpPool.init(allocCb, allocCbData, 1_KB, 2.0f);

	// Set allocators
	g_pool = &m_pool;
	btAlignedAllocSetCustom(btAlloc, btFree);

	// Create objects
	m_broadphase.init();
	m_gpc.init();
	m_broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(m_gpc.get());
	m_filterCallback = anki::newInstance<MyOverlapFilterCallback>(m_pool);
	m_broadphase->getOverlappingPairCache()->setOverlapFilterCallback(m_filterCallback);

	m_collisionConfig.init();

	m_dispatcher.init(m_collisionConfig.get());
	btGImpactCollisionAlgorithm::registerAlgorithm(m_dispatcher.get());

	m_solver.init();

	m_world.init(m_dispatcher.get(), m_broadphase.get(), m_solver.get(), m_collisionConfig.get());
	m_world->setGravity(btVector3(0.0f, -9.8f, 0.0f));

	return Error::kNone;
}

void PhysicsWorld::destroyMarkedForDeletion()
{
	while(true)
	{
		PhysicsObject* obj = nullptr;

		// Don't delete the instance (call the destructor) while holding the lock to avoid deadlocks
		{
			LockGuard<Mutex> lock(m_markedMtx);
			if(!m_markedForDeletion.isEmpty())
			{
				obj = m_markedForDeletion.popFront();
				if(obj->m_registered)
				{
					obj->unregisterFromWorld();
					obj->m_registered = false;
				}
			}
		}

		if(obj == nullptr)
		{
			break;
		}

		deleteInstance(m_pool, obj);
#if ANKI_ENABLE_ASSERTIONS
		const I32 count = m_objectsCreatedCount.fetchSub(1) - 1;
		ANKI_ASSERT(count >= 0);
#endif
	}
}

void PhysicsWorld::update(Second dt)
{
	// First destroy
	destroyMarkedForDeletion();

	// Create new objects
	{
		LockGuard<Mutex> lock(m_markedMtx);

		// Create
		while(!m_markedForCreation.isEmpty())
		{
			PhysicsObject* obj = m_markedForCreation.popFront();
			ANKI_ASSERT(!obj->m_registered);
			obj->registerToWorld();
			obj->m_registered = true;
			m_objectLists[obj->getType()].pushBack(obj);
		}
	}

	// Update the player controllers
	for(PhysicsObject& obj : m_objectLists[PhysicsObjectType::kPlayerController])
	{
		PhysicsPlayerController& playerController = static_cast<PhysicsPlayerController&>(obj);
		playerController.moveToPositionForReal();
	}

	// Update world
	m_world->stepSimulation(F32(dt), 1, 1.0f / 60.0f);

	// Process trigger contacts
	for(PhysicsObject& trigger : m_objectLists[PhysicsObjectType::kTrigger])
	{
		static_cast<PhysicsTrigger&>(trigger).processContacts();
	}

	// Reset the pool
	m_tmpPool.reset();
}

void PhysicsWorld::destroyObject(PhysicsObject* obj)
{
	ANKI_ASSERT(obj);
	LockGuard<Mutex> lock(m_markedMtx);
	if(obj->m_registered)
	{
		m_objectLists[obj->getType()].erase(obj);
	}
	else
	{
		m_markedForCreation.erase(obj);
	}
	m_markedForDeletion.pushBack(obj);
}

void PhysicsWorld::rayCast(WeakArray<PhysicsWorldRayCastCallback*> rayCasts) const
{
	MyRaycastCallback callback;
	for(PhysicsWorldRayCastCallback* cb : rayCasts)
	{
		callback.m_raycast = cb;
		m_world->rayTest(toBt(cb->m_from), toBt(cb->m_to), callback);
	}
}

PhysicsTriggerFilteredPair* PhysicsWorld::getOrCreatePhysicsTriggerFilteredPair(PhysicsTrigger* trigger,
																				PhysicsFilteredObject* filtered,
																				Bool& isNew)
{
	ANKI_ASSERT(trigger && filtered);

	U32 emptySlot = kMaxU32;
	for(U32 i = 0; i < filtered->m_triggerFilteredPairs.getSize(); ++i)
	{
		PhysicsTriggerFilteredPair* pair = filtered->m_triggerFilteredPairs[i];

		if(pair && pair->m_trigger == trigger)
		{
			// Found it
			ANKI_ASSERT(pair->m_filteredObject == filtered);
			isNew = false;
			return pair;
		}
		else if(pair == nullptr)
		{
			// Empty slot, save it for later
			emptySlot = i;
		}
		else if(pair && pair->m_trigger == nullptr)
		{
			// Pair exists but it's invalid, repurpose it
			ANKI_ASSERT(pair->m_filteredObject == filtered);
			emptySlot = i;
		}
	}

	if(emptySlot == kMaxU32)
	{
		ANKI_PHYS_LOGW("Contact ignored. Too many active contacts for the filtered object");
		return nullptr;
	}

	// Not found, create a new one
	isNew = true;

	PhysicsTriggerFilteredPair* newPair;
	if(filtered->m_triggerFilteredPairs[emptySlot] == nullptr)
	{
		filtered->m_triggerFilteredPairs[emptySlot] = anki::newInstance<PhysicsTriggerFilteredPair>(m_pool);
	}
	newPair = filtered->m_triggerFilteredPairs[emptySlot];

	newPair->m_filteredObject = filtered;
	newPair->m_trigger = trigger;
	newPair->m_frame = 0;

	return newPair;
}

} // end namespace anki
