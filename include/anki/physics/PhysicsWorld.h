// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_PHYSICS_PHYSICS_WORLD_H
#define ANKI_PHYSICS_PHYSICS_WORLD_H

#include "anki/physics/Common.h"
#include "anki/physics/PhysicsCollisionShape.h"
#include "anki/physics/PhysicsBody.h"
#include "anki/physics/PhysicsPlayerController.h"
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

	PhysicsPlayerController* newPlayerController(
		const PhysicsPlayerController::Initializer& init)
	{
		return newObjectInternal<PhysicsPlayerController>(
			m_playerControllers, init);
	}

	/// Start asynchronous update.
	Error updateAsync(F32 dt);

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

	const Vec4& getGravity() const
	{
		return m_gravity;
	}

	F32 getDeltaTime() const
	{
		return m_dt;
	}
	/// @}

private:
	ChainAllocator<U8> m_alloc;
	List<PhysicsCollisionShape*> m_collisions;
	List<PhysicsBody*> m_bodies;
	List<PhysicsPlayerController*> m_playerControllers;
	Array<Atomic<U32>, static_cast<U>(PhysicsObject::Type::COUNT)> 
		m_forDeletionCount = {{{0}, {0}, {0}}};
	mutable NewtonWorld* m_world = nullptr;
	Vec4 m_gravity = Vec4(0.0, -9.8, 0.0, 0.0);
	F32 m_dt = 0.0;

	template<typename T, typename TContainer, typename... TArgs>
	T* newObjectInternal(TContainer& cont, TArgs&&... args);

	template<typename T>
	void cleanupMarkedForDeletion(
		List<T*>& container, Atomic<U32>& count);

	/// Custom update
	static void postUpdateCallback(
		const NewtonWorld* const world, 
		void* const listenerUserData, F32 dt)
	{
		static_cast<PhysicsWorld*>(listenerUserData)->postUpdate(dt);
	}

	void postUpdate(F32 dt);

	static void destroyCallback(
		const NewtonWorld* const world, 
		void* const listenerUserData)
	{}
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
