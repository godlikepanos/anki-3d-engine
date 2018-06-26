// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/physics/Common.h>
#include <anki/physics/PhysicsObject.h>
#include <anki/physics/PhysicsWorld.h>

namespace anki
{

void PhysicsPtrDeleter::operator()(PhysicsObject* ptr)
{
	if(ptr == nullptr)
	{
		return;
	}

	auto alloc = ptr->getAllocator();
	ptr->~PhysicsObject();
	alloc.getMemoryPool().free(ptr);
}

} // end namespace anki
