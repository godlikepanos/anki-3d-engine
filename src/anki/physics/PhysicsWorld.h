// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/physics/Common.h>
#include <anki/physics/PhysicsObject.h>
#include <anki/util/List.h>
#include <anki/util/WeakArray.h>

namespace anki
{

/// @addtogroup physics
/// @{

/// Raycast callback (interface).
class PhysicsWorldRayCastCallback
{
public:
	Vec3 m_from;
	Vec3 m_to;
	PhysicsMaterialBit m_materialMask; ///< Materials to check
	Bool8 m_firstHit = true;

	PhysicsWorldRayCastCallback(const Vec3& from, const Vec3& to, PhysicsMaterialBit materialMask)
		: m_from(from)
		, m_to(to)
		, m_materialMask(materialMask)
	{
	}

	/// Process a raycast result.
	virtual void processResult(PhysicsFilteredObject& obj, const Vec3& worldNormal, const Vec3& worldPosition) = 0;
};

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

	void rayCast(WeakArray<PhysicsWorldRayCastCallback*> rayCasts);

	void rayCast(PhysicsWorldRayCastCallback& raycast)
	{
		PhysicsWorldRayCastCallback* ptr = &raycast;
		WeakArray<PhysicsWorldRayCastCallback*> arr(&ptr, 1);
		rayCast(arr);
	}

anki_internal:
	btDynamicsWorld* getBtWorld()
	{
		return m_world.get();
	}

	const btDynamicsWorld* getBtWorld() const
	{
		return m_world.get();
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
	class MyOverlapFilterCallback;
	class MyRaycastCallback;

	HeapAllocator<U8> m_alloc;
	StackAllocator<U8> m_tmpAlloc;

	BtClassWrapper<btDbvtBroadphase> m_broadphase;
	BtClassWrapper<btGhostPairCallback> m_gpc;
	MyOverlapFilterCallback* m_filterCallback = nullptr;

	BtClassWrapper<btDefaultCollisionConfiguration> m_collisionConfig;
	BtClassWrapper<btCollisionDispatcher> m_dispatcher;
	BtClassWrapper<btSequentialImpulseConstraintSolver> m_solver;
	BtClassWrapper<btDiscreteDynamicsWorld> m_world;
	mutable Mutex m_btWorldMtx;

	Array<IntrusiveList<PhysicsObject>, U(PhysicsObjectType::COUNT)> m_objectLists;
	mutable Mutex m_objectListsMtx;
};
/// @}

} // end namespace anki
