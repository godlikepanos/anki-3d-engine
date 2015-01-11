// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_PHYSICS_PHYSICS_BODY_H
#define ANKI_PHYSICS_PHYSICS_BODY_H

#include "anki/physics/PhysicsObject.h"

namespace anki {

/// @addtogroup physics
/// @{

/// Initializer for PhysicsBody.
struct PhysicsBodyInitializer
{
	const PhysicsCollisionShape* m_shape = nullptr;
	F32 m_mass = 0.0;
	Transform m_startTrf = Transform::getIdentity();
	Bool m_kinematic = false;
	Bool m_gravity = true;
};

/// Rigid body.
class PhysicsBody: public PhysicsObject
{
public:
	using Initializer = PhysicsBodyInitializer;

	PhysicsBody(PhysicsWorld* world);

	~PhysicsBody();

	ANKI_USE_RESULT Error create(const Initializer& init);

	const Transform& getTransform(Bool& updated)
	{
		updated = m_updated;
		m_updated = false;
		return m_trf;
	}

	void setTransform(const Transform& trf);

private:
	NewtonBody* m_body = nullptr;
	Transform m_trf = Transform::getIdentity();
	Bool8 m_updated = true;

	/// Newton callback.
	static void onTransform(
		const NewtonBody* const body, 
		const dFloat* const matrix, 
		int threadIndex);

	/// Newton callback
	static void applyGravityForce(
		const NewtonBody* body, 
		dFloat timestep, 
		int threadIndex);
};
/// @}

} // end namespace anki

#endif
