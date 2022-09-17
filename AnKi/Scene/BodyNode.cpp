// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/BodyNode.h>
#include <AnKi/Scene/Components/BodyComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/Components/JointComponent.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Physics/PhysicsWorld.h>
#include <AnKi/Resource/ResourceManager.h>

namespace anki {

/// Body feedback component.
class BodyNode::FeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(BodyNode::FeedbackComponent)

public:
	FeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), true)
	{
	}

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override
	{
		updated = false;

		BodyComponent& bodyc = info.m_node->getFirstComponentOfType<BodyComponent>();

		if(bodyc.getTimestamp() == info.m_node->getGlobalTimestamp())
		{
			MoveComponent& move = info.m_node->getFirstComponentOfType<MoveComponent>();
			move.setLocalTransform(bodyc.getWorldTransform());
		}

		return Error::kNone;
	}
};

ANKI_SCENE_COMPONENT_STATICS(BodyNode::FeedbackComponent)

BodyNode::BodyNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
	newComponent<JointComponent>();
	newComponent<BodyComponent>();
	newComponent<FeedbackComponent>();
	MoveComponent* movec = newComponent<MoveComponent>();
	movec->setIgnoreParentTransform(true);
}

BodyNode::~BodyNode()
{
}

} // end namespace anki
