// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Physics/Common.h>
#include <AnKi/Physics/PhysicsObject.h>
#include <AnKi/Util/List.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Util/ClassWrapper.h>

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
	Bool m_firstHit = true;

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

	ANKI_USE_RESULT Error init(AllocAlignedCallback allocCb, void* allocCbData);

	template<typename T, typename... TArgs>
	PhysicsPtr<T> newInstance(TArgs&&... args)
	{
		T* obj = static_cast<T*>(m_alloc.getMemoryPool().allocate(sizeof(T), alignof(T)));
		::new(obj) T(this, std::forward<TArgs>(args)...);
		{
			LockGuard<Mutex> lock(m_markedMtx);
			m_markedForCreation.pushBack(obj);
		}
#if ANKI_ENABLE_ASSERTS
		const U32 count = m_objectsCreatedCount.fetchAdd(1) + 1;
		ANKI_ASSERT(count > 0);
#endif
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

	StackAllocator<U8> getTempAllocator() const
	{
		return m_tmpAlloc;
	}

	StackAllocator<U8>& getTempAllocator()
	{
		return m_tmpAlloc;
	}

	void rayCast(WeakArray<PhysicsWorldRayCastCallback*> rayCasts) const;

	void rayCast(PhysicsWorldRayCastCallback& raycast) const
	{
		PhysicsWorldRayCastCallback* ptr = &raycast;
		WeakArray<PhysicsWorldRayCastCallback*> arr(&ptr, 1);
		rayCast(arr);
	}

	ANKI_INTERNAL btDynamicsWorld& getBtWorld()
	{
		return *m_world;
	}

	ANKI_INTERNAL const btDynamicsWorld& getBtWorld() const
	{
		return *m_world;
	}

	ANKI_INTERNAL constexpr F32 getCollisionMargin() const
	{
		return 0.04f;
	}

	ANKI_INTERNAL void destroyObject(PhysicsObject* obj);

	ANKI_INTERNAL PhysicsTriggerFilteredPair*
	getOrCreatePhysicsTriggerFilteredPair(PhysicsTrigger* trigger, PhysicsFilteredObject* filtered, Bool& isNew);

private:
	class MyOverlapFilterCallback;
	class MyRaycastCallback;

	HeapAllocator<U8> m_alloc;
	StackAllocator<U8> m_tmpAlloc;

	ClassWrapper<btDbvtBroadphase> m_broadphase;
	ClassWrapper<btGhostPairCallback> m_gpc;
	MyOverlapFilterCallback* m_filterCallback = nullptr;

	ClassWrapper<btDefaultCollisionConfiguration> m_collisionConfig;
	ClassWrapper<btCollisionDispatcher> m_dispatcher;
	ClassWrapper<btSequentialImpulseConstraintSolver> m_solver;
	ClassWrapper<btDiscreteDynamicsWorld> m_world;

	Array<IntrusiveList<PhysicsObject>, U(PhysicsObjectType::COUNT)> m_objectLists;
	IntrusiveList<PhysicsObject> m_markedForCreation;
	IntrusiveList<PhysicsObject> m_markedForDeletion;
	Mutex m_markedMtx; ///< Locks the above

#if ANKI_ENABLE_ASSERTS
	Atomic<I32> m_objectsCreatedCount = {0};
#endif

	void destroyMarkedForDeletion();
};
/// @}

} // end namespace anki
