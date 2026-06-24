// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Physics/PhysicsJoint.h>

namespace anki {

enum class JointComponentyType : U8
{
	kPoint,
	kHinge,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(JointComponentyType)

inline constexpr Array<const Char*, U32(JointComponentyType::kCount)> kJointComponentTypeName = {"Point", "Hinge"};

// Defines a single joint that connects the parent node with the node of this component.
class JointComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(JointComponent)

public:
	JointComponent(const SceneComponentInitInfo& init);

	~JointComponent();

	void setJointType(JointComponentyType type)
	{
		if(ANKI_EXPECT(type < JointComponentyType::kCount))
		{
			m_type = type;
		}
	}

	JointComponentyType getJointType() const
	{
		return m_type;
	}

	Bool isValid() const;

	// Pivot manipulation
	Vec3 getPivot1Origin() const
	{
		return m_pivot1.getOrigin().xyz;
	}

	void setPivot1Origin(Vec3 origin)
	{
		m_pivot1.setOrigin(origin);
		m_pivotsDirty = true;
	}

	Vec3 getPivot2Origin() const
	{
		return m_pivot2.getOrigin().xyz;
	}

	void setPivot2Origin(Vec3 origin)
	{
		m_pivot2.setOrigin(origin);
		m_pivotsDirty = true;
	}

	Mat3 getPivot1Rotation() const
	{
		return m_pivot1.getRotation().getRotationPart();
	}

	void setPivot1Rotation(const Mat3& rotation)
	{
		m_pivot1.setRotation(rotation);
		m_pivotsDirty = true;
	}

	Mat3 getPivot2Rotation() const
	{
		return m_pivot2.getRotation().getRotationPart();
	}

	void setPivot2Rotation(const Mat3& rotation)
	{
		m_pivot2.setRotation(rotation);
		m_pivotsDirty = true;
	}

	// It's a deferred operation
	void movePivot1ToPivot2()
	{
		m_movePivot1To2 = true;
		m_movePivot2To1 = false;
	}

	// It's a deferred operation
	void movePivot2ToPivot1()
	{
		m_movePivot1To2 = false;
		m_movePivot2To1 = true;
	}
	// End pivot manipulation

private:
	PhysicsJointPtr m_joint;

	SceneNode* m_node = nullptr;

	U32 m_parentNodeUuid = 0;

	Transform m_pivot1 = Transform::getIdentity();
	Transform m_pivot2 = Transform::getIdentity();

	JointComponentyType m_type = JointComponentyType::kPoint;

	Bool m_pivotsDirty : 1 = true;

	// Deferred ops
	Bool m_movePivot1To2 : 1 = false;
	Bool m_movePivot2To1 : 1 = false;

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;

	Error serialize(SceneSerializer& serializer) override;
};

} // end namespace anki
