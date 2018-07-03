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
	F32 m_friction = 0.5f;
};

/// Rigid body.
class PhysicsBody : public PhysicsFilteredObject
{
	ANKI_PHYSICS_OBJECT

public:
	static const PhysicsObjectType CLASS_TYPE = PhysicsObjectType::BODY;

	const Transform& getTransform() const
	{
		return m_trf;
	}

	void setTransform(const Transform& trf)
	{
		m_trf = trf;
		m_body->setWorldTransform(toBt(trf));
	}

	void applyForce(const Vec3& force, const Vec3& relPos)
	{
		m_body->applyForce(toBt(force), toBt(relPos));
	}

	void setMass(F32 mass);

	F32 getMass() const
	{
		return m_mass;
	}

anki_internal:
	btRigidBody* getBtBody() const
	{
		ANKI_ASSERT(m_body);
		return m_body;
	}

private:
	class MotionState;

	PhysicsCollisionShapePtr m_shape;
	MotionState* m_motionState = nullptr;
	btRigidBody* m_body = nullptr;

	Transform m_trf = Transform::getIdentity();
	F32 m_mass = 1.0f;

	PhysicsBody(PhysicsWorld* world, const PhysicsBodyInitInfo& init);

	~PhysicsBody();
};
/// @}

} // end namespace anki
