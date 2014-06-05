// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/physics/PhysicsObject.h"
#include "anki/physics/PhysicsWorld.h"

namespace anki {

//==============================================================================
SceneAllocator<U8> PhysicsObject::getSceneAllocator() const
{
	return world->getSceneAllocator();
}

} // end namespace anki

