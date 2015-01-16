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
	F32 m_stepHeight = 1.9 * 0.33;
};

/// A player controller that walks the world.
class PhysicsPlayerController final: public PhysicsObject
{
public:
	using Initializer = PhysicsPlayerControllerInitializer;

	PhysicsPlayerController(PhysicsWorld* world)
	:	PhysicsObject(Type::PLAYER_CONTROLLER, world)
	{}

	ANKI_USE_RESULT Error create(const Initializer& init);

	void setVelocity(F32 forwardSpeed, F32 strafeSpeed, F32 jumpSpeed, 
		const Vec3& forwardDir, const Vec3& gravity, F32 dt);

private:
	Vec4 m_upDir;
	Vec4 m_frontDir;
	Vec4 m_groundPlane;
	Vec4 m_groundVeloc;
	F32 m_innerRadius;
	F32 m_outerRadius;
	F32 m_height;
	F32 m_stepHeight;
	F32 m_maxSlope;
	F32 m_sphereCastOrigin;
	F32 m_restrainingDistance;
	Bool8 m_isJumping;
	NewtonCollision* m_castingShape;
	NewtonCollision* m_supportShape;
	NewtonCollision* m_upperBodyShape;
	NewtonBody* m_body;

	static constexpr F32 MIN_RESTRAINING_DISTANCE = 1.0e-2;

	void setClimbSlope(F32 ang)
	{
		ANKI_ASSERT(ang >= 0.0);
		m_maxSlope = cos(ang);
	}

	void postUpdate();

	/// Called by Newton thread to update the controller.
	static void postUpdateKernelCallback(
		NewtonWorld* const world, 
		void* const context, 
		int threadIndex);
};
/// @}

} // end namespace anki

#endif

