// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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
	F32 m_mass = 0.0f;
	Transform m_startTrf = Transform::getIdentity();
	Bool m_kinematic = false;
	Bool m_gravity = true;
	F32 m_friction = 0.5f;
};

/// Rigid body.
class PhysicsBody : public PhysicsObject
{
public:
	PhysicsBody(PhysicsWorld* world, const PhysicsBodyInitInfo& init);

	~PhysicsBody();

	const Transform& getTransform(Bool& updated)
	{
		updated = m_updated;
		m_updated = false;
		return m_trf;
	}

	void setTransform(const Transform& trf)
	{
		m_trf = trf;
		m_body->setWorldTransform(toBt(trf));
	}

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

	void applyForce(const Vec3& force, const Vec3& relPos)
	{
		m_body->applyForce(toBt(force), toBt(relPos));
	}

anki_internal:
	btRigidBody* getBtBody() const
	{
		ANKI_ASSERT(m_body);
		return m_body;
	}

private:
	class MotionState;

	MotionState* m_motionState = nullptr;
	btRigidBody* m_body = nullptr;

	Transform m_trf = Transform::getIdentity();
	F32 m_friction = 0.03f;
	F32 m_elasticity = 0.1f;
	PhysicsMaterialBit m_materialBits = PhysicsMaterialBit::ALL;
	Bool8 m_updated = true;
};
/// @}

} // end namespace anki
