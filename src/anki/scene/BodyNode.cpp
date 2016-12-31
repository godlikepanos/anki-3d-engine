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

	ANKI_USE_RESULT Error update(SceneNode& node, F32, F32, Bool& updated)
	{
		updated = false;

		BodyComponent& bodyc = node.getComponent<BodyComponent>();

		if(bodyc.getTimestamp() == node.getGlobalTimestamp())
		{
			MoveComponent& move = node.getComponent<MoveComponent>();
			move.setLocalTransform(bodyc.getTransform());
		}

		return ErrorCode::NONE;
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
	SceneComponent* comp;

	// Load resource
	ANKI_CHECK(getResourceManager().loadResource(resourceFname, m_rsrc));

	// Create body
	PhysicsBodyInitInfo init;
	init.m_mass = 1.0;
	init.m_shape = m_rsrc->getShape();
	m_body = getSceneGraph().getPhysicsWorld().newInstance<PhysicsBody>(init);

	// Body component
	comp = getSceneAllocator().newInstance<BodyComponent>(this, m_body);
	addComponent(comp, true);

	// Feedback component
	comp = getSceneAllocator().newInstance<BodyFeedbackComponent>(this);
	addComponent(comp, true);

	// Move component
	comp = getSceneAllocator().newInstance<MoveComponent>(this);
	addComponent(comp, true);

	return ErrorCode::NONE;
}

} // end namespace anki
