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

/// Joint base class. Joints connect two (or a single one) rigid bodies together.
class PhysicsJoint : public PhysicsObject
{
public:
	static const PhysicsObjectType CLASS_TYPE = PhysicsObjectType::JOINT;

	/// Set the breaking impulse.
	void setBreakingImpulseThreshold(F32 impulse)
	{
		m_joint->setBreakingImpulseThreshold(impulse);
	}

	/// Return true if the joint broke.
	Bool isBroken() const
	{
		return !m_joint->isEnabled();
	}

	/// Break the joint.
	void brake()
	{
		m_joint->setEnabled(false);
	}

protected:
	btTypedConstraint* m_joint = nullptr;
	PhysicsBodyPtr m_bodyA;
	PhysicsBodyPtr m_bodyB;

	PhysicsJoint(PhysicsWorld* world);

	~PhysicsJoint();

	void addToWorld();
};

/// Point to point joint.
class PhysicsPoint2PointJoint : public PhysicsJoint
{
	ANKI_PHYSICS_OBJECT

private:
	PhysicsPoint2PointJoint(PhysicsWorld* world, PhysicsBodyPtr bodyA, const Vec3& relPos);

	PhysicsPoint2PointJoint(
		PhysicsWorld* world, PhysicsBodyPtr bodyA, const Vec3& relPosA, PhysicsBodyPtr bodyB, const Vec3& relPosB);
};

/// Hinge joint.
class PhysicsHingeJoint : public PhysicsJoint
{
	ANKI_PHYSICS_OBJECT

private:
	PhysicsHingeJoint(PhysicsWorld* world, PhysicsBodyPtr bodyA, const Vec3& relPos, const Vec3& axis);

	PhysicsHingeJoint(PhysicsWorld* world,
		PhysicsBodyPtr bodyA,
		const Vec3& relPosA,
		const Vec3& axisA,
		PhysicsBodyPtr bodyB,
		const Vec3& relPosB,
		const Vec3& axisB);
};
/// @}

} // end namespace anki