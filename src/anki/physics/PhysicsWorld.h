// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/physics/Common.h>
#include <anki/util/List.h>

namespace anki
{

/// @addtogroup physics
/// @{

/// The master container for all physics related stuff.
class PhysicsWorld
{
public:
	PhysicsWorld();
	~PhysicsWorld();

	ANKI_USE_RESULT Error create(AllocAlignedCallback allocCb, void* allocCbData);

	template<typename T, typename... TArgs>
	PhysicsPtr<T> newInstance(TArgs&&... args)
	{
		PhysicsPtr<T> out;
		T* ptr = m_alloc.template newInstance<T>(this, std::forward<TArgs>(args)...);
		out.reset(ptr);
		return out;
	}

	/// Do the update.
	Error update(Second dt);

	HeapAllocator<U8> getAllocator()
	{
		return m_alloc;
	}

anki_internal:
	btDynamicsWorld* getBtWorld() const
	{
		ANKI_ASSERT(m_world);
		return m_world;
	}

	void deleteObjectDeferred(PhysicsObject* obj)
	{
		LockGuard<Mutex> lock(m_mtx);
		m_forDeletion.pushBack(m_alloc, obj);
	}

	F32 getCollisionMargin() const
	{
		return 0.04;
	}

private:
	HeapAllocator<U8> m_alloc;

	btBroadphaseInterface* m_broadphase = nullptr;
	btDefaultCollisionConfiguration* m_collisionConfig = nullptr;
	btCollisionDispatcher* m_dispatcher = nullptr;
	btSequentialImpulseConstraintSolver* m_solver = nullptr;
	btDiscreteDynamicsWorld* m_world = nullptr;

	Mutex m_mtx;
	List<PhysicsObject*> m_forDeletion;

	template<typename T, typename... TArgs>
	PhysicsPtr<T> newObjectInternal(TArgs&&... args);

	void cleanupMarkedForDeletion();
};
/// @}

} // end namespace anki
