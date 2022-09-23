// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Physics/PhysicsObject.h>
#include <AnKi/Util/ClassWrapper.h>

namespace anki {

/// @addtogroup physics
/// @{

/// Joint base class. Joints connect two (or a single one) rigid bodies together.
class PhysicsJoint : public PhysicsObject
{
	ANKI_PHYSICS_OBJECT(PhysicsObjectType::kJoint)

public:
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
		ClassWrapper<btPoint2PointConstraint> m_p2p;
		ClassWrapper<btHingeConstraint> m_hinge;
	};

	PhysicsBodyPtr m_bodyA;
	PhysicsBodyPtr m_bodyB;

	enum class JointType : U8
	{
		kP2P,
		kHinge,
	};

	JointType m_type;

	PhysicsJoint(PhysicsWorld* world, JointType type);

	void registerToWorld() override;
	void unregisterFromWorld() override;

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
		case JointType::kP2P:
			return m_p2p.get();
		case JointType::kHinge:
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
	ANKI_PHYSICS_OBJECT(PhysicsObjectType::kJoint)

private:
	PhysicsPoint2PointJoint(PhysicsWorld* world, PhysicsBodyPtr bodyA, const Vec3& relPos);

	PhysicsPoint2PointJoint(PhysicsWorld* world, PhysicsBodyPtr bodyA, const Vec3& relPosA, PhysicsBodyPtr bodyB,
							const Vec3& relPosB);

	~PhysicsPoint2PointJoint();
};

/// Hinge joint.
class PhysicsHingeJoint : public PhysicsJoint
{
	ANKI_PHYSICS_OBJECT(PhysicsObjectType::kJoint)

private:
	PhysicsHingeJoint(PhysicsWorld* world, PhysicsBodyPtr bodyA, const Vec3& relPos, const Vec3& axis);

	PhysicsHingeJoint(PhysicsWorld* world, PhysicsBodyPtr bodyA, const Vec3& relPosA, const Vec3& axisA,
					  PhysicsBodyPtr bodyB, const Vec3& relPosB, const Vec3& axisB);

	~PhysicsHingeJoint();
};
/// @}

} // end namespace anki
