// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/BodyNode.h>
#include <anki/scene/BodyComponent.h>
#include <anki/scene/MoveComponent.h>
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

	ANKI_USE_RESULT Error update(SceneNode& node, Second, Second, Bool& updated) override
	{
		updated = false;

		BodyComponent& bodyc = node.getComponent<BodyComponent>();

		if(bodyc.getTimestamp() == node.getGlobalTimestamp())
		{
			MoveComponent& move = node.getComponent<MoveComponent>();
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
	init.m_mass = 1.0;
	init.m_shape = m_rsrc->getShape();
	m_body = getSceneGraph().getPhysicsWorld().newInstance<PhysicsBody>(init);

	// Body component
	newComponent<BodyComponent>(this, m_body);

	// Feedback component
	newComponent<BodyFeedbackComponent>(this);

	// Move component
	newComponent<MoveComponent>(this);

	return Error::NONE;
}

} // end namespace anki
