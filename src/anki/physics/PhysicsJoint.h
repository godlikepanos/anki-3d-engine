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
		getJoint()->setBreakingImpulseThreshold(impulse);
	}

	/// Return true if the joint broke.
	Bool isBroken() const
	{
		return !getJoint()->isEnabled();
	}

	/// Break the joint.
	void brake()
	{
		getJoint()->setEnabled(false);
	}

protected:
	union
	{
		BtClassWrapper<btPoint2PointConstraint> m_p2p;
		BtClassWrapper<btHingeConstraint> m_hinge;
	};

	PhysicsBodyPtr m_bodyA;
	PhysicsBodyPtr m_bodyB;

	enum class JointType : U8
	{
		P2P,
		HINGE,
	};

	JointType m_type;

	PhysicsJoint(PhysicsWorld* world, JointType type);

	void addToWorld();
	void removeFromWorld();

	const btTypedConstraint* getJoint() const
	{
		return getJointInternal();
	}

	btTypedConstraint* getJoint()
	{
		return const_cast<btTypedConstraint*>(getJointInternal());
	}

	const btTypedConstraint* getJointInternal() const
	{
		switch(m_type)
		{
		case JointType::P2P:
			return m_p2p.get();
		case JointType::HINGE:
			return m_hinge.get();
		default:
			ANKI_ASSERT(0);
			return nullptr;
		}
	}
};

/// Point to point joint.
class PhysicsPoint2PointJoint : public PhysicsJoint
{
	ANKI_PHYSICS_OBJECT

private:
	PhysicsPoint2PointJoint(PhysicsWorld* world, PhysicsBodyPtr bodyA, const Vec3& relPos);

	PhysicsPoint2PointJoint(
		PhysicsWorld* world, PhysicsBodyPtr bodyA, const Vec3& relPosA, PhysicsBodyPtr bodyB, const Vec3& relPosB);

	~PhysicsPoint2PointJoint();
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

	~PhysicsHingeJoint();
};
/// @}

} // end namespace anki