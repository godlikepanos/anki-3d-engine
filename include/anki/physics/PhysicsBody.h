// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_PHYSICS_PHYSICS_BODY_H
#define ANKI_PHYSICS_PHYSICS_BODY_H

#include "anki/physics/Common.h"

namespace anki {

/// @addtogroup physics
/// @{

/// Body initializer.
struct PhysicsBodyInitializer
{
	PhysicsWorld* m_world = nullptr;
	PhysicsCollisionShape* m_shape = nullptr;
	F32 m_mass = 0.0;
	Transform m_startTrf = Transform::getIdentity();
	Bool m_kinematic = false;
	Bool m_gravity = true;
};

/// Rigid body.
class PhysicsBody
{
public:
	using Initializer = PhysicsBodyInitializer;

	PhysicsBody();

	~PhysicsBody();

	ANKI_USE_RESULT Error create(const Initializer& init);

private:
	NewtonBody* m_body = nullptr;
	Transform m_trf = Transform::getIdentity();

	// Newton callback.
	static void onTransform(
		const NewtonBody* const body, 
		const dFloat* const matrix, 
		int threadIndex);
};
/// @}

} // end namespace anki

#endif
