// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/physics/Common.h>
#include <anki/physics/PhysicsObject.h>
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
		void* mem = m_alloc.getMemoryPool().allocate(sizeof(T), alignof(T));
		::new(mem) T(this, std::forward<TArgs>(args)...);

		T* obj = static_cast<T*>(mem);
		LockGuard<Mutex> lock(m_objectListsMtx);
		m_objectLists[obj->getType()].pushBack(obj);

		return PhysicsPtr<T>(obj);
	}

	/// Do the update.
	Error update(Second dt);

	HeapAllocator<U8> getAllocator() const
	{
		return m_alloc;
	}

	HeapAllocator<U8>& getAllocator()
	{
		return m_alloc;
	}

anki_internal:
	btDynamicsWorld* getBtWorld() const
	{
		ANKI_ASSERT(m_world);
		return m_world;
	}

	F32 getCollisionMargin() const
	{
		return 0.04f;
	}

	ANKI_USE_RESULT LockGuard<Mutex> lockBtWorld() const
	{
		return LockGuard<Mutex>(m_btWorldMtx);
	}

	void destroyObject(PhysicsObject* obj);

private:
	HeapAllocator<U8> m_alloc;
	StackAllocator<U8> m_tmpAlloc;

	btBroadphaseInterface* m_broadphase = nullptr;
	btGhostPairCallback* m_gpc = nullptr;

	btDefaultCollisionConfiguration* m_collisionConfig = nullptr;
	btCollisionDispatcher* m_dispatcher = nullptr;
	btSequentialImpulseConstraintSolver* m_solver = nullptr;
	btDiscreteDynamicsWorld* m_world = nullptr;
	mutable Mutex m_btWorldMtx;

	Array<IntrusiveList<PhysicsObject>, U(PhysicsObjectType::COUNT)> m_objectLists;
	mutable Mutex m_objectListsMtx;
};
/// @}

} // end namespace anki
