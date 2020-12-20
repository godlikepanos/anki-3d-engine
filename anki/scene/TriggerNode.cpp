// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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
public:
	MoveFeedbackComponent()
		: SceneComponent(SceneComponentType::NONE)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) final
	{
		updated = false;

		const MoveComponent& move = node.getFirstComponentOfType<MoveComponent>();
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			node.getFirstComponentOfType<TriggerComponent>().setTransform(move.getWorldTransform());
		}

		return Error::NONE;
	}
};

TriggerNode::TriggerNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
}

TriggerNode::~TriggerNode()
{
}

Error TriggerNode::init(F32 sphereRadius)
{
	newComponent<MoveComponent>();
	newComponent<MoveFeedbackComponent>();
	TriggerComponent* triggerc = newComponent<TriggerComponent>(this);
	triggerc->setSphere(sphereRadius);

	return Error::NONE;
}

} // end namespace anki
