// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_PHYSICS_PSYSICS_OBJECT_H
#define ANKI_PHYSICS_PSYSICS_OBJECT_H

#include "anki/util/Assert.h"
#include "anki/scene/Common.h"

namespace anki {

class PhysicsWorld;

// The base of all physics objects
class PhysicsObject
{
public:
	PhysicsObject(PhysicsWorld* world_)
		: world(world_)
	{
		ANKI_ASSERT(world);
	}

	virtual ~PhysicsObject()
	{}

	/// @name Accessors
	/// @{
	PhysicsWorld& getPhysicsWorld()
	{
		return *world;
	}
	const PhysicsWorld& getPhysicsWorld() const
	{
		return *world;
	}

	SceneAllocator<U8> getSceneAllocator() const;
	/// @}

private:
	PhysicsWorld* world;
};

} // end namespace anki

#endif
