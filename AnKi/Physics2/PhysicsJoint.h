// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Physics2/Common.h>
#include <AnKi/Util/ClassWrapper.h>

namespace anki {
namespace v2 {

/// @addtogroup physics
/// @{

/// Wrapper on top of Jolt joints.
class PhysicsJoint : public PhysicsObjectBase
{
	ANKI_PHYSICS_COMMON_FRIENDS
	friend class PhysicsJointPtrDeleter;

private:
	enum class Type : U8
	{
		kPoint,
		kHinge,

		kCount
	};

	union
	{
		ClassWrapper<JPH::TwoBodyConstraint> m_base;
		ClassWrapper<JPH::PointConstraint> m_point;
		ClassWrapper<JPH::HingeConstraint> m_hinge;
	};

	PhysicsBodyPtr m_body1;
	PhysicsBodyPtr m_body2;

	U32 m_arrayIndex = kMaxU32;
	Type m_type;

	PhysicsJoint(Type type)
		: PhysicsObjectBase(PhysicsObjectType::kJoint, nullptr)
		, m_type(type)
	{
		ANKI_ASSERT(type < Type::kCount);
		ANKI_ASSERT(&m_base == static_cast<JPH::TwoBodyConstraint*>(&m_point));
		ANKI_ASSERT(&m_base == static_cast<JPH::TwoBodyConstraint*>(&m_hinge));
	}

	~PhysicsJoint()
	{
		m_base.destroy();
	}
};
/// @}

} // namespace v2
} // end namespace anki
