// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/DecalNode.h>
#include <AnKi/Scene/Components/DecalComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/Components/SpatialComponent.h>

namespace anki {

/// Decal feedback component.
class DecalNode::MoveFeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(DecalNode::MoveFeedbackComponent)

public:
	MoveFeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), true)
	{
	}

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override
	{
		updated = false;

		MoveComponent& movec = info.m_node->getFirstComponentOfType<MoveComponent>();
		if(movec.getTimestamp() == info.m_node->getGlobalTimestamp())
		{
			static_cast<DecalNode&>(*info.m_node).onMove(movec);
		}

		return Error::kNone;
	}
};

ANKI_SCENE_COMPONENT_STATICS(DecalNode::MoveFeedbackComponent)

/// Decal feedback component.
class DecalNode::ShapeFeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(DecalNode::ShapeFeedbackComponent)

public:
	ShapeFeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), true)
	{
	}

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override
	{
		updated = false;

		DecalComponent& decalc = info.m_node->getFirstComponentOfType<DecalComponent>();

		if(decalc.getTimestamp() == info.m_node->getGlobalTimestamp())
		{
			static_cast<DecalNode&>(*info.m_node).onDecalUpdated(decalc);
		}

		return Error::kNone;
	}
};

ANKI_SCENE_COMPONENT_STATICS(DecalNode::ShapeFeedbackComponent)

DecalNode::DecalNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
	newComponent<MoveComponent>();
	newComponent<MoveFeedbackComponent>();
	newComponent<DecalComponent>();
	newComponent<ShapeFeedbackComponent>();
	newComponent<SpatialComponent>();
}

DecalNode::~DecalNode()
{
}

void DecalNode::onMove(MoveComponent& movec)
{
	getFirstComponentOfType<SpatialComponent>().setSpatialOrigin(movec.getWorldTransform().getOrigin().xyz());
	getFirstComponentOfType<DecalComponent>().setWorldTransform(movec.getWorldTransform());
}

void DecalNode::onDecalUpdated(DecalComponent& decalc)
{
	getFirstComponentOfType<SpatialComponent>().setObbWorldSpace(decalc.getBoundingVolumeWorldSpace());
}

} // end namespace anki
