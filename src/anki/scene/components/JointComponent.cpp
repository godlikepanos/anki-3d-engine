// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/JointComponent.h>
#include <anki/scene/components/BodyComponent.h>
#include <anki/scene/SceneGraph.h>
#include <anki/physics/PhysicsWorld.h>

namespace anki
{

JointComponent::~JointComponent()
{
	m_jointList.destroy(getAllocator());
}

void JointComponent::newPoint2PointJoint(const Vec3& relPosA)
{
	BodyComponent* bodyc = m_node->tryGetComponent<BodyComponent>();

	if(bodyc)
	{
		PhysicsJointPtr joint =
			getSceneGraph().getPhysicsWorld().newInstance<PhysicsPoint2PointJoint>(bodyc->getPhysicsBody(), relPosA);

		m_jointList.pushBack(getAllocator(), joint);
	}
	else
	{
		ANKI_SCENE_LOGW("Can't create new joint. The node is missing a body component");
	}
}

} // end namespace anki