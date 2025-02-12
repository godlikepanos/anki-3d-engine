// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Physics/Common.h>
#include <AnKi/Util/ClassWrapper.h>

namespace anki {

/// @addtogroup physics
/// @{

/// Wrapper on top of Jolt joints.
class PhysicsJoint : public PhysicsObjectBase
{
	ANKI_PHYSICS_COMMON_FRIENDS
	friend class PhysicsJointPtrDeleter;

private:
	union
	{
		ClassWrapper<JPH::TwoBodyConstraint> m_base;
		ClassWrapper<JPH::PointConstraint> m_point;
		ClassWrapper<JPH::HingeConstraint> m_hinge;
	};

	PhysicsBodyPtr m_body1;
	PhysicsBodyPtr m_body2;

	PhysicsJoint()
		: PhysicsObjectBase(PhysicsObjectType::kJoint)
	{
		ANKI_ASSERT(&m_base == static_cast<JPH::TwoBodyConstraint*>(&m_point));
		ANKI_ASSERT(&m_base == static_cast<JPH::TwoBodyConstraint*>(&m_hinge));
	}

	~PhysicsJoint()
	{
		m_base.destroy();
	}
};
/// @}

} // end namespace anki
