// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/TriggerNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/TriggerComponent.h>
#include <anki/physics/PhysicsWorld.h>
#include <anki/physics/PhysicsCollisionShape.h>
#include <anki/physics/PhysicsTrigger.h>

namespace anki
{

class TriggerNode::MoveFeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(TriggerNode::MoveFeedbackComponent)

public:
	MoveFeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), true)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) final
	{
		updated = false;

		const MoveComponent& move = node.getFirstComponentOfType<MoveComponent>();
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			node.getFirstComponentOfType<TriggerComponent>().setWorldTransform(move.getWorldTransform());
		}

		return Error::NONE;
	}
};

ANKI_SCENE_COMPONENT_STATICS(TriggerNode::MoveFeedbackComponent)

TriggerNode::TriggerNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
	newComponent<MoveComponent>();
	newComponent<MoveFeedbackComponent>();
	newComponent<TriggerComponent>();
}

TriggerNode::~TriggerNode()
{
}

} // end namespace anki
