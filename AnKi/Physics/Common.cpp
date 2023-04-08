// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Physics/Common.h>
#include <AnKi/Physics/PhysicsObject.h>
#include <AnKi/Physics/PhysicsWorld.h>

namespace anki {

void PhysicsPtrDeleter::operator()(PhysicsObject* ptr)
{
	if(ptr == nullptr)
	{
		return;
	}

	PhysicsWorld::getSingleton().destroyObject(ptr);
}

} // end namespace anki
