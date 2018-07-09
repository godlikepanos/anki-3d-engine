// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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
	MoveFeedbackComponent(SceneNode* node)
		: SceneComponent(SceneComponentType::NONE, node)
	{
	}

	ANKI_USE_RESULT Error update(Second, Second, Bool& updated) final
	{
		updated = false;

		const MoveComponent& move = m_node->getComponent<MoveComponent>();
		if(move.getTimestamp() == m_node->getGlobalTimestamp())
		{
			TriggerNode* node = static_cast<TriggerNode*>(m_node);
			node->m_trigger->setTransform(move.getWorldTransform());
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
	m_shape = getSceneGraph().getPhysicsWorld().newInstance<PhysicsSphere>(sphereRadius);
	m_trigger = getSceneGraph().getPhysicsWorld().newInstance<PhysicsTrigger>(m_shape);
	m_trigger->setUserData(this);

	newComponent<MoveComponent>();
	newComponent<MoveFeedbackComponent>();
	newComponent<TriggerComponent>(m_trigger);

	return Error::NONE;
}

} // end namespace anki