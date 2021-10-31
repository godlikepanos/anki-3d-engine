// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Physics/Common.h>
#include <AnKi/Util/List.h>

namespace anki {

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
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(PhysicsObjectType)

#define ANKI_PHYSICS_OBJECT_FRIENDS \
	friend class PhysicsWorld; \
	friend class PhysicsPtrDeleter; \
	template<typename T, typename TDeleter> \
	friend class IntrusivePtr;

#define ANKI_PHYSICS_OBJECT(type) \
	ANKI_PHYSICS_OBJECT_FRIENDS \
public: \
	static constexpr PhysicsObjectType CLASS_TYPE = type; \
\
private:

/// Base of all physics objects.
class PhysicsObject : public IntrusiveListEnabled<PhysicsObject>
{
	ANKI_PHYSICS_OBJECT_FRIENDS

public:
	PhysicsObject(PhysicsObjectType type, PhysicsWorld* world)
		: m_world(world)
		, m_type(type)
	{
		ANKI_ASSERT(m_world);
	}

	virtual ~PhysicsObject()
	{
		ANKI_ASSERT(!m_registered);
	}

	PhysicsObjectType getType() const
	{
		return m_type;
	}

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

private:
	Bool m_registered = false;

	virtual void registerToWorld() = 0;

	virtual void unregisterFromWorld() = 0;

private:
	Atomic<I32> m_refcount = {0};
	PhysicsObjectType m_type;
	void* m_userData = nullptr;
};

/// This is a factor that will decide if two filtered objects will be checked for collision.
/// @memberof PhysicsFilteredObject
class PhysicsBroadPhaseFilterCallback
{
public:
	virtual Bool needsCollision(const PhysicsFilteredObject& a, const PhysicsFilteredObject& b) = 0;
};

/// A pair of a trigger and a filtered object.
class PhysicsTriggerFilteredPair
{
public:
	PhysicsFilteredObject* m_filteredObject = nullptr;
	PhysicsTrigger* m_trigger = nullptr;
	U64 m_frame = 0;

	Bool isAlive() const
	{
		return m_filteredObject && m_trigger;
	}

	Bool shouldDelete() const
	{
		return m_filteredObject == nullptr && m_trigger == nullptr;
	}
};

/// A PhysicsObject that takes part into collision detection. Has functionality to filter the broad phase detection.
class PhysicsFilteredObject : public PhysicsObject
{
public:
	ANKI_PHYSICS_OBJECT_FRIENDS

	PhysicsFilteredObject(PhysicsObjectType type, PhysicsWorld* world)
		: PhysicsObject(type, world)
	{
	}

	~PhysicsFilteredObject();

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

	static constexpr U32 MAX_TRIGGER_FILTERED_PAIRS = 4;
	Array<PhysicsTriggerFilteredPair*, MAX_TRIGGER_FILTERED_PAIRS> m_triggerFilteredPairs = {};
};
/// @}

} // end namespace anki
