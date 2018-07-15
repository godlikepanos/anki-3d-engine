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

/// Type of the physics object.
enum class PhysicsObjectType : U8
{
	COLLISION_SHAPE,
	JOINT,
	BODY,
	PLAYER_CONTROLLER,
	TRIGGER,

	COUNT,
	FIRST = 0,
	LAST = COUNT - 1,
	FIRST_FILTERED = BODY,
	LAST_FILTERED = TRIGGER,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(PhysicsObjectType, inline)

/// Base of all physics objects.
class PhysicsObject : public IntrusiveListEnabled<PhysicsObject>
{
public:
	PhysicsObject(PhysicsObjectType type, PhysicsWorld* world)
		: m_world(world)
		, m_type(type)
	{
		ANKI_ASSERT(m_world);
	}

	virtual ~PhysicsObject()
	{
	}

	PhysicsObjectType getType() const
	{
		return m_type;
	}

	PhysicsWorld& getWorld()
	{
		return *m_world;
	}

	const PhysicsWorld& getWorld() const
	{
		return *m_world;
	}

	Atomic<I32>& getRefcount()
	{
		return m_refcount;
	}

	HeapAllocator<U8> getAllocator() const;

	void setUserData(void* ud)
	{
		m_userData = ud;
	}

	void* getUserData() const
	{
		return m_userData;
	}

protected:
	PhysicsWorld* m_world = nullptr;

private:
	Atomic<I32> m_refcount = {0};
	PhysicsObjectType m_type;
	void* m_userData = nullptr;
};

#define ANKI_PHYSICS_OBJECT \
	friend class PhysicsWorld; \
	friend class PhysicsPtrDeleter;

/// This is a factor that will decide if two filtered objects will be checked for collision.
/// @memberof PhysicsFilteredObject
class PhysicsBroadPhaseFilterCallback
{
public:
	virtual Bool needsCollision(const PhysicsFilteredObject& a, const PhysicsFilteredObject& b) = 0;
};

/// A PhysicsObject that takes part into collision detection. Has functionality to filter the broad phase detection.
class PhysicsFilteredObject : public PhysicsObject
{
public:
	PhysicsFilteredObject(PhysicsObjectType type, PhysicsWorld* world)
		: PhysicsObject(type, world)
	{
	}

	~PhysicsFilteredObject()
	{
	}

	static Bool classof(const PhysicsObject* obj)
	{
		return obj->getType() >= PhysicsObjectType::FIRST_FILTERED
			   && obj->getType() <= PhysicsObjectType::LAST_FILTERED;
	}

	/// Get the material(s) this object belongs.
	PhysicsMaterialBit getMaterialGroup() const
	{
		return m_materialGroup;
	}

	/// Set the material(s) this object belongs.
	void setMaterialGroup(PhysicsMaterialBit bits)
	{
		m_materialGroup = bits;
	}

	/// Get the materials this object collides.
	PhysicsMaterialBit getMaterialMask() const
	{
		return m_materialMask;
	}

	/// Set the materials this object collides.
	void setMaterialMask(PhysicsMaterialBit bit)
	{
		m_materialMask = bit;
	}

	/// Get the broadphase callback.
	PhysicsBroadPhaseFilterCallback* getPhysicsBroadPhaseFilterCallback() const
	{
		return m_filter;
	}

	/// Set the broadphase callback.
	void setPhysicsBroadPhaseFilterCallback(PhysicsBroadPhaseFilterCallback* f)
	{
		m_filter = f;
	}

private:
	PhysicsMaterialBit m_materialGroup = PhysicsMaterialBit::ALL;
	PhysicsMaterialBit m_materialMask = PhysicsMaterialBit::ALL;

	PhysicsBroadPhaseFilterCallback* m_filter = nullptr;
};
/// @}

} // end namespace anki
