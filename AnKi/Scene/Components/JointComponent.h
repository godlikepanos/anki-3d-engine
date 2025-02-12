// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Physics2/PhysicsJoint.h>

namespace anki {

/// @addtogroup scene
/// @{

/// @memberof JointComponent
enum class JointType : U8
{
	kPoint,
	kHinge,

	kCount,
	kFirst
};

/// Contains a single joint that connects the parent node with the 1st child node of the node that has this component.
class JointComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(JointComponent)

public:
	JointComponent(SceneNode* node);

	~JointComponent();

	void setType(JointType type)
	{
		m_type = type;
	}

private:
	v2::PhysicsJointPtr m_joint;

	U32 m_parentNodeUuid = 0;
	U32 m_childNodeUuid = 0;

	JointType m_type = JointType::kCount;

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override;
};
/// @}

} // end namespace anki
