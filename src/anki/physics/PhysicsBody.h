// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/physics/PhysicsObject.h>

namespace anki
{

/// @addtogroup physics
/// @{

/// Init info for PhysicsBody.
class PhysicsBodyInitInfo
{
public:
	PhysicsCollisionShapePtr m_shape;
	F32 m_mass = 0.0;
	Transform m_startTrf = Transform::getIdentity();
	Bool m_kinematic = false;
	Bool m_gravity = true;
	Bool m_static = false;
};

/// Rigid body.
class PhysicsBody : public PhysicsObject
{
public:
	PhysicsBody(PhysicsWorld* world);

	~PhysicsBody();

	ANKI_USE_RESULT Error create(const PhysicsBodyInitInfo& init);

	const Transform& getTransform(Bool& updated)
	{
		updated = m_updated;
		m_updated = false;
		return m_trf;
	}

	void setTransform(const Transform& trf);

	F32 getFriction() const
	{
		return m_friction;
	}

	void setFriction(F32 friction)
	{
		m_friction = friction;
	}

	F32 getElasticity() const
	{
		return m_elasticity;
	}

	void setElasticity(F32 elasticity)
	{
		m_elasticity = elasticity;
	}

	void setMaterialsThatCollide(PhysicsMaterialBit bits)
	{
		m_materialBits = bits;
	}

private:
	NewtonBody* m_body = nullptr;
	void* m_sceneCollisionProxy = nullptr;
	Transform m_trf = Transform::getIdentity();
	F32 m_friction = 0.03;
	F32 m_elasticity = 0.1;
	PhysicsMaterialBit m_materialBits = PhysicsMaterialBit::ALL;
	Bool8 m_updated = true;

	/// Newton callback.
	static void onTransformCallback(const NewtonBody* const body, const dFloat* const matrix, int threadIndex);

	/// Newton callback
	static void applyGravityForce(const NewtonBody* body, dFloat timestep, int threadIndex);
};
/// @}

} // end namespace anki
