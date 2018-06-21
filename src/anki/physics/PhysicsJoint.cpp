// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/physics/PhysicsJoint.h>
#include <anki/physics/PhysicsBody.h>
#include <anki/physics/PhysicsWorld.h>

namespace anki
{

PhysicsJoint::PhysicsJoint(PhysicsWorld* world)
	: PhysicsObject(PhysicsObjectType::JOINT, world)
{
}

PhysicsJoint::~PhysicsJoint()
{
	if(m_joint)
	{
		getWorld().getBtWorld()->removeConstraint(m_joint);
		getAllocator().deleteInstance(m_joint);
	}
}

PhysicsPoint2PointJoint::PhysicsPoint2PointJoint(PhysicsWorld* world, PhysicsBodyPtr bodyA, const Vec3& relPos)
	: PhysicsJoint(world)
{
	m_bodyA = bodyA;
	m_joint = getAllocator().newInstance<btPoint2PointConstraint>(*m_bodyA->getBtBody(), toBt(relPos));
	m_joint->setUserConstraintPtr(static_cast<PhysicsJoint*>(this));

	getWorld().getBtWorld()->addConstraint(m_joint);
}

} // end namespace anki