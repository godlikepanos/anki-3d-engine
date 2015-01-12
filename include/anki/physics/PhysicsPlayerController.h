// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_PHYSICS_PLAYER_CONTROLLER_H
#define ANKI_PHYSICS_PLAYER_CONTROLLER_H

#include "anki/physics/PhysicsObject.h"

namespace anki {

/// @addtogroup physics
/// @{

/// Initializer for PhysicsPlayerController.
struct PhysicsPlayerControllerInitializer
{
	F32 m_mass = 83.0;
	F32 m_innerRadius = 0.30;
	F32 m_outerRadius = 0.50;
	F32 m_height = 1.9;
	F32 m_stepHeight = 0.5;
};

/// A player controller that walks the world.
class PhysicsPlayerController final: public PhysicsObject
{
public:
	using Initializer = PhysicsPlayerControllerInitializer;

	PhysicsPlayerController(PhysicsWorld* world)
	:	PhysicsObject(Type::PLAYER_CONTROLLER, world)
	{}

	ANKI_USE_RESULT Error create(PhysicsWorld* world, const Initializer& init);

	void setVelocity(F32 forwardSpeed, F32 strafeSpeed, F32 jumpSpeed, 
		const Vec3& forwardDir, const Vec3& gravity, F32 dt);
};
/// @}

} // end namespace anki

#endif

