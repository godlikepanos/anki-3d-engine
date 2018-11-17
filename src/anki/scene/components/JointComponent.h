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
		: SceneComponent(CLASS_TYPE)
		, m_node(node)
	{
		ANKI_ASSERT(node);
	}

	~JointComponent();

	/// Create a point 2 point joint on the BodyComponent of the SceneNode.
	void newPoint2PointJoint(const Vec3& relPosFactor, F32 brakingImpulse = MAX_F32);

	/// Create a point 2 point joint on the BodyComponents of the SceneNode and its child node.
	void newPoint2PointJoint2(const Vec3& relPosFactorA, const Vec3& relPosFactorB, F32 brakingImpulse = MAX_F32);

	/// Create a hinge joint on the BodyComponent of the SceneNode.
	void newHingeJoint(const Vec3& relPosFactor, const Vec3& axis, F32 brakingImpulse = MAX_F32);

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override;

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