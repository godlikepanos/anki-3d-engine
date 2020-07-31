// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/physics/PhysicsJoint.h>
#include <anki/physics/PhysicsBody.h>
#include <anki/physics/PhysicsWorld.h>

namespace anki
{

PhysicsJoint::PhysicsJoint(PhysicsWorld* world, JointType type)
	: PhysicsObject(CLASS_TYPE, world)
	, m_type(type)
{
}

void PhysicsJoint::addToWorld()
{
	getJoint()->setUserConstraintPtr(static_cast<PhysicsObject*>(this));

	auto lock = getWorld().lockBtWorld();
	getWorld().getBtWorld()->addConstraint(getJoint());
}

void PhysicsJoint::removeFromWorld()
{
	auto lock = getWorld().lockBtWorld();
	getWorld().getBtWorld()->removeConstraint(getJoint());
}

PhysicsPoint2PointJoint::PhysicsPoint2PointJoint(PhysicsWorld* world, PhysicsBodyPtr bodyA, const Vec3& relPos)
	: PhysicsJoint(world, JointType::P2P)
{
	m_bodyA = bodyA;
	m_p2p.init(*m_bodyA->getBtBody(), toBt(relPos));
	addToWorld();
}

PhysicsPoint2PointJoint::PhysicsPoint2PointJoint(PhysicsWorld* world, PhysicsBodyPtr bodyA, const Vec3& relPosA,
												 PhysicsBodyPtr bodyB, const Vec3& relPosB)
	: PhysicsJoint(world, JointType::P2P)
{
	ANKI_ASSERT(bodyA != bodyB);
	m_bodyA = bodyA;
	m_bodyB = bodyB;

	m_p2p.init(*m_bodyA->getBtBody(), *m_bodyB->getBtBody(), toBt(relPosA), toBt(relPosB));
	addToWorld();
}

PhysicsPoint2PointJoint::~PhysicsPoint2PointJoint()
{
	removeFromWorld();
	m_p2p.destroy();
}

PhysicsHingeJoint::PhysicsHingeJoint(PhysicsWorld* world, PhysicsBodyPtr bodyA, const Vec3& relPos, const Vec3& axis)
	: PhysicsJoint(world, JointType::HINGE)
{
	m_bodyA = bodyA;
	m_hinge.init(*m_bodyA->getBtBody(), toBt(relPos), toBt(axis));
	addToWorld();
}

PhysicsHingeJoint::~PhysicsHingeJoint()
{
	removeFromWorld();
	m_hinge.destroy();
}

} // end namespace anki