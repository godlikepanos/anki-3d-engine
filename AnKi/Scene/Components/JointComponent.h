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

// Contains a single joint that connects the parent node with the 1st child node of the node that has this component.
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

private:
	PhysicsJointPtr m_joint;

	SceneNode* m_node = nullptr;

	U32 m_parentNodeUuid = 0;
	U32 m_childNodeUuid = 0;

	JointComponentyType m_type = JointComponentyType::kPoint;

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;
};

} // end namespace anki
