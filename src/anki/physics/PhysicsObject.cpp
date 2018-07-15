// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/physics/PhysicsObject.h>
#include <anki/physics/PhysicsWorld.h>

namespace anki
{

HeapAllocator<U8> PhysicsObject::getAllocator() const
{
	return m_world->getAllocator();
}

} // end namespace anki