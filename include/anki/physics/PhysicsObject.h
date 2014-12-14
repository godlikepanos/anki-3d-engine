// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_PHYSICS_PHYSICS_OBJECT_H
#define ANKI_PHYSICS_PHYSICS_OBJECT_H

#include "anki/physics/Common.h"

namespace anki {

/// @addtogroup physics
/// @{

/// Base of all physics objects.
class PhysicsObject
{
public:
	PhysicsObject(PhysicsWorld* world)
	:	m_world(world)
	{
		ANKI_ASSERT(m_world);
	}

	virtual ~PhysicsObject()
	{
		ANKI_ASSERT(m_markedForDeletion == true);
	}

	void setMarkedForDeletion();

	Bool getMarkedForDeletion() const
	{
		return m_markedForDeletion;
	}

protected:
	PhysicsWorld* m_world = nullptr;

private:
	Bool8 m_markedForDeletion = false;
};
/// @}

} // end namespace anki

#endif
