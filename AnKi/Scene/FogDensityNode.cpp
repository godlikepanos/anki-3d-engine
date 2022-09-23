// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/FogDensityNode.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/Components/FogDensityComponent.h>
#include <AnKi/Scene/Components/SpatialComponent.h>

namespace anki {

class FogDensityNode::MoveFeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(FogDensityNode::MoveFeedbackComponent)

public:
	MoveFeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), true)
	{
	}

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override
	{
		updated = false;

		const MoveComponent& movec = info.m_node->getFirstComponentOfType<MoveComponent>();
		if(movec.getTimestamp() == info.m_node->getGlobalTimestamp())
		{
			static_cast<FogDensityNode&>(*info.m_node).onMoveUpdated(movec);
		}

		return Error::kNone;
	}
};

ANKI_SCENE_COMPONENT_STATICS(FogDensityNode::MoveFeedbackComponent)

class FogDensityNode::DensityShapeFeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(FogDensityNode::DensityShapeFeedbackComponent)

public:
	DensityShapeFeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), true)
	{
	}

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override
	{
		updated = false;

		const FogDensityComponent& fogc = info.m_node->getFirstComponentOfType<FogDensityComponent>();
		if(fogc.getTimestamp() == info.m_node->getGlobalTimestamp())
		{
			static_cast<FogDensityNode&>(*info.m_node).onDensityShapeUpdated(fogc);
		}

		return Error::kNone;
	}
};

ANKI_SCENE_COMPONENT_STATICS(FogDensityNode::DensityShapeFeedbackComponent)

FogDensityNode::FogDensityNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
	newComponent<MoveComponent>();
	newComponent<MoveFeedbackComponent>();
	newComponent<FogDensityComponent>();
	newComponent<DensityShapeFeedbackComponent>();
	newComponent<SpatialComponent>();
}

FogDensityNode::~FogDensityNode()
{
}

void FogDensityNode::onMoveUpdated(const MoveComponent& movec)
{
	getFirstComponentOfType<FogDensityComponent>().setWorldPosition(movec.getWorldTransform().getOrigin().xyz());
	getFirstComponentOfType<SpatialComponent>().setSpatialOrigin(movec.getWorldTransform().getOrigin().xyz());
}

void FogDensityNode::onDensityShapeUpdated(const FogDensityComponent& fogc)
{
	SpatialComponent& spatialc = getFirstComponentOfType<SpatialComponent>();

	if(fogc.isAabb())
	{
		spatialc.setAabbWorldSpace(fogc.getAabbWorldSpace());
	}
	else
	{
		ANKI_ASSERT(fogc.isSphere());
		spatialc.setSphereWorldSpace(fogc.getSphereWorldSpace());
	}
}

} // end namespace anki
