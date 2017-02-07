// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/physics/Common.h>
#include <anki/util/List.h>

namespace anki
{

// Forward
class CharacterControllerManager;

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
	PhysicsPtr<T> newInstance(TArgs&&... args);

	/// Start asynchronous update.
	Error updateAsync(F32 dt);

	/// End asynchronous update.
	void waitUpdate();

	const Vec4& getGravity() const
	{
		return m_gravity;
	}

anki_internal:
	NewtonWorld* getNewtonWorld() const
	{
		ANKI_ASSERT(m_world);
		return m_world;
	}

	/// Used for static collision.
	NewtonCollision* getNewtonScene() const
	{
		return m_sceneCollision;
	}

	F32 getDeltaTime() const
	{
		return m_dt;
	}

	void deleteObjectDeferred(PhysicsObject* obj)
	{
		LockGuard<Mutex> lock(m_mtx);
		m_forDeletion.pushBack(m_alloc, obj);
	}

	CharacterControllerManager& getCharacterControllerManager()
	{
		ANKI_ASSERT(m_playerManager);
		return *m_playerManager;
	}

private:
	HeapAllocator<U8> m_alloc;
	mutable NewtonWorld* m_world = nullptr;
	NewtonCollision* m_sceneCollision = nullptr;
	NewtonBody* m_sceneBody = nullptr;
	Vec4 m_gravity = Vec4(0.0, -9.8, 0.0, 0.0);
	F32 m_dt = 0.0;

	/// @note Don't delete it. Newton will
	CharacterControllerManager* m_playerManager = nullptr;

	Mutex m_mtx;
	List<PhysicsObject*> m_forDeletion;

	template<typename T, typename... TArgs>
	PhysicsPtr<T> newObjectInternal(TArgs&&... args);

	void cleanupMarkedForDeletion();

	static void destroyCallback(const NewtonWorld* const world, void* const listenerUserData)
	{
	}

	void registerObject(PhysicsObject* ptr);

	static int onAabbOverlapCallback(const NewtonMaterial* const material,
		const NewtonBody* const body0,
		const NewtonBody* const body1,
		int threadIndex)
	{
		(void)material;
		(void)body0;
		(void)body1;
		(void)threadIndex;
		return 1;
	}

	static void onContactCallback(const NewtonJoint* contactJoint, F32 timestep, int threadIndex);
};

template<typename T, typename... TArgs>
inline PhysicsPtr<T> PhysicsWorld::newInstance(TArgs&&... args)
{
	Error err = ErrorCode::NONE;
	PhysicsPtr<T> out;

	T* ptr = m_alloc.template newInstance<T>(this);
	err = ptr->create(std::forward<TArgs>(args)...);

	if(!err)
	{
		registerObject(ptr);
		out.reset(ptr);
	}
	else
	{
		ANKI_PHYS_LOGE("Failed to create physics object");

		if(ptr)
		{
			m_alloc.deleteInstance(ptr);
		}
	}

	return out;
}
/// @}

} // end namespace anki
