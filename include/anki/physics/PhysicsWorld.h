// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_PHYSICS_PHYSICS_WORLD_H
#define ANKI_PHYSICS_PHYSICS_WORLD_H

#include "anki/physics/Common.h"

namespace anki {

/// @addtogroup physics
/// @{

/// The master container for all physics related stuff.
class PhysicsWorld
{
public:
	PhysicsWorld();
	~PhysicsWorld();

	ANKI_USE_RESULT Error create(SceneGraph* scene);

	NewtonWorld* _getNewtonWorld()
	{
		ANKI_ASSERT(m_world);
		return m_world;
	}

private:
	SceneGraph* m_scene = nullptr;
	NewtonWorld* m_world = nullptr;
};

/// @}

} // end namespace anki

#endif
