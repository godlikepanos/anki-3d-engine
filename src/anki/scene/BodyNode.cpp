// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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
class BodyFeedbackComponent : public SceneComponent
{
public:
	BodyFeedbackComponent(SceneNode* node)
		: SceneComponent(SceneComponentType::NONE, node)
	{
	}

	ANKI_USE_RESULT Error update(Second, Second, Bool& updated) override
	{
		updated = false;

		BodyComponent& bodyc = m_node->getComponent<BodyComponent>();

		if(bodyc.getTimestamp() == m_node->getGlobalTimestamp())
		{
			MoveComponent& move = m_node->getComponent<MoveComponent>();
			move.setLocalTransform(bodyc.getTransform());
		}

		return Error::NONE;
	}
};

BodyNode::BodyNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
}

BodyNode::~BodyNode()
{
}

Error BodyNode::init(const CString& resourceFname)
{
	// Load resource
	ANKI_CHECK(getResourceManager().loadResource(resourceFname, m_rsrc));

	// Create body
	PhysicsBodyInitInfo init;
	init.m_mass = 1.0f;
	init.m_shape = m_rsrc->getShape();
	m_body = getSceneGraph().getPhysicsWorld().newInstance<PhysicsBody>(init);
	m_body->setUserData(this);

	// Joint component
	newComponent<JointComponent>();

	// Body component
	newComponent<BodyComponent>(m_body);

	// Feedback component
	newComponent<BodyFeedbackComponent>();

	// Move component
	newComponent<MoveComponent>(MoveComponentFlag::IGNORE_PARENT_TRANSFORM);

	return Error::NONE;
}

} // end namespace anki
