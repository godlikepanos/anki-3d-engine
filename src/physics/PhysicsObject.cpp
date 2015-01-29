// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/physics/PhysicsObject.h"
#include "anki/physics/PhysicsWorld.h"

namespace anki {

//==============================================================================
void PhysicsObject::setMarkedForDeletion()
{
	if(!m_markedForDeletion)
	{
		m_markedForDeletion = true;
		m_world->_increaseObjectsMarkedForDeletion(m_type);
	}
}

} // end namespace anki

