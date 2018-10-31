// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/FogDensityNode.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/FogDensityComponent.h>

namespace anki
{

class FogDensityNode::FeedbackComponent : public SceneComponent
{
public:
	FeedbackComponent(SceneNode* node)
		: SceneComponent(SceneComponentType::NONE, node)
	{
	}

	ANKI_USE_RESULT Error update(Second, Second, Bool& updated) override
	{
		updated = false;

		MoveComponent& movec = m_node->getComponent<MoveComponent>();
		if(movec.getTimestamp() == m_node->getGlobalTimestamp())
		{
			FogDensityComponent& fogc = m_node->getComponent<FogDensityComponent>();
			fogc.updatePosition(movec.getWorldTransform().getOrigin());
		}

		return Error::NONE;
	}
};

FogDensityNode::FogDensityNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
	// Create components
	newComponent<MoveComponent>(MoveComponentFlag::NONE);
	newComponent<FeedbackComponent>();
	newComponent<FogDensityComponent>();
}

} // end namespace anki