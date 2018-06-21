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

class PhysicsJoint : public PhysicsObject
{
public:
	PhysicsJoint(PhysicsWorld* world);

	~PhysicsJoint();

protected:
	btTypedConstraint* m_joint = nullptr;
	PhysicsBodyPtr m_bodyA;
	PhysicsBodyPtr m_bodyB;
};

class PhysicsPoint2PointJoint : public PhysicsJoint
{
public:
	PhysicsPoint2PointJoint(PhysicsWorld* world, PhysicsBodyPtr bodyA, const Vec3& relPos);

	PhysicsPoint2PointJoint(PhysicsWorld* world, PhysicsBodyPtr bodyA, PhysicsBodyPtr bodyB);
};
/// @}

} // end namespace anki