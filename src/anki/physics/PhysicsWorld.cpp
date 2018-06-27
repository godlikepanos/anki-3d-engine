// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/physics/PhysicsWorld.h>
#include <anki/physics/PhysicsCollisionShape.h>
#include <anki/physics/PhysicsBody.h>
#include <anki/physics/PhysicsTrigger.h>
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

	m_alloc.deleteInstance(m_world);
	m_alloc.deleteInstance(m_solver);
	m_alloc.deleteInstance(m_dispatcher);
	m_alloc.deleteInstance(m_collisionConfig);
	m_alloc.deleteInstance(m_broadphase);
	m_alloc.deleteInstance(m_gpc);

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
	m_broadphase = m_alloc.newInstance<btDbvtBroadphase>();
	m_gpc = m_alloc.newInstance<btGhostPairCallback>();
	m_broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(m_gpc);

	m_collisionConfig = m_alloc.newInstance<btDefaultCollisionConfiguration>();

	m_dispatcher = m_alloc.newInstance<btCollisionDispatcher>(m_collisionConfig);
	btGImpactCollisionAlgorithm::registerAlgorithm(m_dispatcher);

	m_solver = m_alloc.newInstance<btSequentialImpulseConstraintSolver>();

	m_world = m_alloc.newInstance<btDiscreteDynamicsWorld>(m_dispatcher, m_broadphase, m_solver, m_collisionConfig);
	m_world->setGravity(btVector3(0.0f, -9.8f, 0.0f));

	return Error::NONE;
}

Error PhysicsWorld::update(Second dt)
{
	// Update world
	{
		auto lock = lockBtWorld();
		m_world->stepSimulation(dt, 2, 1.0 / 60.0);
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

} // end namespace anki
