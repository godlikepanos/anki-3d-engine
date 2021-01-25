// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/BodyNode.h>
#include <anki/scene/components/BodyComponent.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/JointComponent.h>
#include <anki/scene/SceneGraph.h>
#include <anki/physics/PhysicsWorld.h>
#include <anki/resource/ResourceManager.h>

namespace anki
{

/// Body feedback component.
class BodyNode::FeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(BodyNode::FeedbackComponent)

public:
	FeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), true)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		BodyComponent& bodyc = node.getFirstComponentOfType<BodyComponent>();

		if(bodyc.getTimestamp() == node.getGlobalTimestamp())
		{
			MoveComponent& move = node.getFirstComponentOfType<MoveComponent>();
			move.setLocalTransform(bodyc.getWorldTransform());
		}

		return Error::NONE;
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
