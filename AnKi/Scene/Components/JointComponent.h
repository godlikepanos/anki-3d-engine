// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Physics/PhysicsJoint.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Contains a list of joints.
class JointComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(JointComponent)

public:
	JointComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId())
		, m_node(node)
	{
		ANKI_ASSERT(node);
	}

	~JointComponent();

	/// Create a point 2 point joint on the BodyComponent of the SceneNode.
	void newPoint2PointJoint(const Vec3& relPosFactor, F32 brakingImpulse = kMaxF32);

	/// Create a point 2 point joint on the BodyComponents of the SceneNode and its child node.
	void newPoint2PointJoint2(const Vec3& relPosFactorA, const Vec3& relPosFactorB, F32 brakingImpulse = kMaxF32);

	/// Create a hinge joint on the BodyComponent of the SceneNode.
	void newHingeJoint(const Vec3& relPosFactor, const Vec3& axis, F32 brakingImpulse = kMaxF32);

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override;

private:
	class JointNode;

	SceneNode* m_node;
	IntrusiveList<JointNode> m_jointList;

	/// Given a 3 coodrinates that lie in [-1.0, +1.0] compute a pivot point that lies into the AABB of the collision
	/// shape of the body.
	static Vec3 computeLocalPivotFromFactors(const PhysicsBodyPtr& body, const Vec3& factors);

	void removeAllJoints();

	template<typename TJoint, typename... TArgs>
	void newJoint(const Vec3& relPosFactor, F32 brakingImpulse, TArgs&&... args);
};
/// @}

} // end namespace anki
