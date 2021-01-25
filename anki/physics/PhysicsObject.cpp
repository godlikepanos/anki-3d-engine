// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/physics/PhysicsObject.h>
#include <anki/physics/PhysicsWorld.h>

namespace anki
{

PhysicsFilteredObject::~PhysicsFilteredObject()
{
	for(PhysicsTriggerFilteredPair* pair : m_triggerFilteredPairs)
	{
		if(pair == nullptr)
		{
			continue;
		}

		pair->m_filteredObject = nullptr;

		if(pair->shouldDelete())
		{
			getAllocator().deleteInstance(pair);
		}
	}
}

HeapAllocator<U8> PhysicsObject::getAllocator() const
{
	return m_world->getAllocator();
}

} // end namespace anki
