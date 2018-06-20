// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/physics/Common.h>

namespace anki
{

/// @addtogroup physics
/// @{

/// Type of the physics object.
enum class PhysicsObjectType : U8
{
	COLLISION_SHAPE,
	BODY,
	JOINT,
	PLAYER_CONTROLLER,
	COUNT
};

/// Base of all physics objects.
class PhysicsObject
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

protected:
	PhysicsWorld* m_world = nullptr;

private:
	Atomic<I32> m_refcount = {0};
	PhysicsObjectType m_type;
};
/// @}

} // end namespace anki
