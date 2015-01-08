// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_PHYSICS_PHYSICS_WORLD_H
#define ANKI_PHYSICS_PHYSICS_WORLD_H

#include "anki/physics/Common.h"
#include "anki/physics/PhysicsCollisionShape.h"
#include "anki/physics/PhysicsBody.h"
#include "anki/util/List.h"

namespace anki {

/// @addtogroup physics
/// @{

/// The master container for all physics related stuff.
class PhysicsWorld
{
public:
	PhysicsWorld();
	~PhysicsWorld();

	ANKI_USE_RESULT Error create(
		AllocAlignedCallback allocCb, void* allocCbData);

	template<typename T, typename... TArgs>
	T* newCollisionShape(TArgs&&... args)
	{
		return newObjectInternal<T>(m_collisions, std::forward<TArgs>(args)...);
	}

	template<typename T, typename... TArgs>
	T* newBody(TArgs&&... args)
	{
		return newObjectInternal<T>(m_bodies, std::forward<TArgs>(args)...);
	}

	/// Start asynchronous update.
	Error updateAsync(F32 timestep);

	/// End asynchronous update.
	void waitUpdate();

	/// @privatesection
	/// @{
	NewtonWorld* _getNewtonWorld() const
	{
		ANKI_ASSERT(m_world);
		return m_world;
	}

	void _increaseObjectsMarkedForDeletion(PhysicsObject::Type type);
	/// @}

private:
	ChainAllocator<U8> m_alloc;
	List<PhysicsCollisionShape*> m_collisions;
	List<PhysicsBody*> m_bodies;
	Array<AtomicU32, static_cast<U>(PhysicsObject::Type::COUNT)> 
		m_forDeletionCount = {{{0}, {0}, {0}}};
	mutable NewtonWorld* m_world = nullptr;

	template<typename T, typename TContainer, typename... TArgs>
	T* newObjectInternal(TContainer& cont, TArgs&&... args);

	template<typename T>
	void cleanupMarkedForDeletion(
		List<T*>& container, AtomicU32& count);
};

//==============================================================================
template<typename T, typename TContainer, typename... TArgs>
inline T* PhysicsWorld::newObjectInternal(TContainer& cont, TArgs&&... args)
{
	Error err = ErrorCode::NONE;
	ChainAllocator<T> al = m_alloc;

	T* ptr = al.template newInstance<T>(this);
	if(ptr)
	{
		err = ptr->create(std::forward<TArgs>(args)...);
	}
	else
	{
		err = ErrorCode::OUT_OF_MEMORY;
	}

	if(!err)
	{
		err = cont.pushBack(m_alloc, ptr);
	}

	if(err)
	{
		ANKI_LOGE("Failed to create physics object");

		if(ptr)
		{
			al.deleteInstance(ptr);
			ptr = nullptr;
		}
	}

	return ptr;
}
/// @}

} // end namespace anki

#endif
