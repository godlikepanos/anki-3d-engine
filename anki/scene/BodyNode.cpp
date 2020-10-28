// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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
public:
	FeedbackComponent()
		: SceneComponent(SceneComponentType::NONE)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		BodyComponent& bodyc = node.getFirstComponentOfType<BodyComponent>();

		if(bodyc.getTimestamp() == node.getGlobalTimestamp())
		{
			MoveComponent& move = node.getFirstComponentOfType<MoveComponent>();
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
	newComponent<JointComponent>(this);

	// Body component
	newComponent<BodyComponent>(m_body);

	// Feedback component
	newComponent<FeedbackComponent>();

	// Move component
	newComponent<MoveComponent>(MoveComponentFlag::IGNORE_PARENT_TRANSFORM);

	return Error::NONE;
}

} // end namespace anki
