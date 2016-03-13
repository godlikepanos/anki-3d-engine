// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/physics/Common.h>

namespace anki
{

/// @addtogroup physics
/// @{

/// Base of all physics objects.
class PhysicsObject
{
public:
	/// Type of the physics object.
	enum class Type : U8
	{
		COLLISION_SHAPE,
		BODY,
		JOINT,
		PLAYER_CONTROLLER,
		COUNT
	};

	PhysicsObject(Type type, PhysicsWorld* world)
		: m_world(world)
		, m_type(type)
		, m_refcount(0)
	{
		ANKI_ASSERT(m_world);
	}

	virtual ~PhysicsObject()
	{
	}

	Type getType() const
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

protected:
	PhysicsWorld* m_world = nullptr;

private:
	Type m_type;
	Atomic<I32> m_refcount;
};
/// @}

} // end namespace anki
