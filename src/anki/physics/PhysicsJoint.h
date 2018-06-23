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
	PhysicsJoint(PhysicsWorld* world);

	~PhysicsJoint();

	void setBreakingImpulseThreshold(F32 impulse)
	{
		m_joint->setBreakingImpulseThreshold(impulse);
	}

protected:
	btTypedConstraint* m_joint = nullptr;
	PhysicsBodyPtr m_bodyA;
	PhysicsBodyPtr m_bodyB;

	void addToWorld();
};

/// Point 2 point joint.
class PhysicsPoint2PointJoint : public PhysicsJoint
{
public:
	PhysicsPoint2PointJoint(PhysicsWorld* world, PhysicsBodyPtr bodyA, const Vec3& relPos);

	PhysicsPoint2PointJoint(PhysicsWorld* world, PhysicsBodyPtr bodyA, PhysicsBodyPtr bodyB);
};

/// Hinge joint.
class PhysicsHingeJoint : public PhysicsJoint
{
public:
	PhysicsHingeJoint(PhysicsWorld* world, PhysicsBodyPtr bodyA, const Vec3& relPos, const Vec3& axis);
};
/// @}

} // end namespace anki