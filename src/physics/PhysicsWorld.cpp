// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/physics/PhysicsWorld.h"

namespace anki {

//==============================================================================
// Ugly but there is no other way
static ChainAllocator<U8>* gAlloc = nullptr;

static void* newtonAlloc(int size)
{
	return gAlloc->allocate(size);
}

static void newtonFree(void* const ptr, int size)
{
	gAlloc->deallocate(ptr, size);
}

//==============================================================================
PhysicsWorld::PhysicsWorld()
{}

//==============================================================================
PhysicsWorld::~PhysicsWorld()
{
	if(m_world)
	{
		NewtonDestroy(m_world);
	}

	gAlloc = nullptr;
}

//==============================================================================
Error PhysicsWorld::create(AllocAlignedCallback allocCb, void* allocCbData)
{
	Error err = ErrorCode::NONE;

	m_alloc = ChainAllocator<U8>(
		allocCb, allocCbData, 
		1024 * 10,
		1024 * 1024,
		ChainMemoryPool::ChunkGrowMethod::MULTIPLY,
		2);
	
	// Set allocators
	gAlloc = &m_alloc;
	NewtonSetMemorySystem(newtonAlloc, newtonFree);

	// Initialize world
	m_world = NewtonCreate();
	if(!m_world)
	{
		ANKI_LOGE("NewtonCreate() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	return err;
}

//==============================================================================
void PhysicsWorld::_increaseObjectsMarkedForDeletion(PhysicsObject::Type type)
{
	++m_forDeletionCount[static_cast<U>(type)];
}

//==============================================================================
Error PhysicsWorld::updateAsync(F32 timestep)
{
	// Do cleanup of marked for deletion
	cleanupMarkedForDeletion(m_bodies, 
		m_forDeletionCount[static_cast<U>(PhysicsObject::Type::BODY)]);
	cleanupMarkedForDeletion(m_collisions, m_forDeletionCount[
		static_cast<U>(PhysicsObject::Type::COLLISION_SHAPE)]);

	// Update
	NewtonUpdateAsync(m_world, timestep);

	return ErrorCode::NONE;
}

//==============================================================================
void PhysicsWorld::waitUpdate()
{
	NewtonWaitForUpdateToFinish(m_world);
}

//==============================================================================
template<typename T>
void PhysicsWorld::cleanupMarkedForDeletion(
	List<T*>& container, AtomicU32& count)
{
	while(count > 0)
	{
		Bool found = false;
		auto it = container.begin();
		auto end = container.end();
		for(; it != end; it++)
		{
			if((*it)->getMarkedForDeletion())
			{
				// Delete node
				container.erase(m_alloc, it);
				m_alloc.deleteInstance(*it);
				found = true;
				break;
			}
		}

		(void)found;
		ANKI_ASSERT(found && "Something is wrong with marked for deletion");
	}
}

} // end namespace anki
