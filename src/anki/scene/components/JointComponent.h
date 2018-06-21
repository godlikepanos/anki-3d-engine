// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/components/SceneComponent.h>
#include <anki/physics/PhysicsJoint.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Contains a list of joints.
class JointComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::JOINT;

	JointComponent(SceneNode* node)
		: SceneComponent(SceneComponentType::JOINT, node)
	{
	}

	~JointComponent();

	void newPoint2PointJoint(const Vec3& relPosA);

private:
	List<PhysicsJointPtr> m_jointList;
};
/// @}

} // end namespace anki