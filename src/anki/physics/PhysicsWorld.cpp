// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/physics/PhysicsWorld.h>
#include <anki/physics/PhysicsPlayerController.h>
#include <anki/physics/PhysicsCollisionShape.h>
#include <anki/physics/PhysicsBody.h>

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
	cleanupMarkedForDeletion();

	m_alloc.deleteInstance(m_world);
	m_alloc.deleteInstance(m_solver);
	m_alloc.deleteInstance(m_dispatcher);
	m_alloc.deleteInstance(m_collisionConfig);
	m_alloc.deleteInstance(m_broadphase);

	gAlloc = nullptr;
}

Error PhysicsWorld::create(AllocAlignedCallback allocCb, void* allocCbData)
{
	m_alloc = HeapAllocator<U8>(allocCb, allocCbData);

	// Set allocators
	gAlloc = &m_alloc;
	btAlignedAllocSetCustom(btAlloc, btFree);

	// Create objects
	m_broadphase = m_alloc.newInstance<btDbvtBroadphase>();
	m_collisionConfig = m_alloc.newInstance<btDefaultCollisionConfiguration>();
	m_dispatcher = m_alloc.newInstance<btCollisionDispatcher>(m_collisionConfig);
	m_solver = m_alloc.newInstance<btSequentialImpulseConstraintSolver>();
	m_world = m_alloc.newInstance<btDiscreteDynamicsWorld>(m_dispatcher, m_broadphase, m_solver, m_collisionConfig);

	return Error::NONE;
}

Error PhysicsWorld::update(Second dt)
{
	// Do cleanup of marked for deletion
	cleanupMarkedForDeletion();

	// Update
	m_world->stepSimulation(dt, 1, 1.0 / 60.0);

	return Error::NONE;
}

void PhysicsWorld::cleanupMarkedForDeletion()
{
	LockGuard<Mutex> lock(m_mtx);

	while(!m_forDeletion.isEmpty())
	{
		auto it = m_forDeletion.getBegin();
		PhysicsObject* obj = *it;

		// Remove from objects marked for deletion
		m_forDeletion.erase(m_alloc, it);

		// Finaly, delete it
		m_alloc.deleteInstance(obj);
	}
}

} // end namespace anki
