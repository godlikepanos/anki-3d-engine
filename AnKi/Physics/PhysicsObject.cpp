// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Physics/PhysicsObject.h>
#include <AnKi/Physics/PhysicsWorld.h>

namespace anki {

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
			deleteInstance(getMemoryPool(), pair);
		}
	}
}

HeapMemoryPool& PhysicsObject::getMemoryPool()
{
	return m_world->getMemoryPool();
}

} // end namespace anki
