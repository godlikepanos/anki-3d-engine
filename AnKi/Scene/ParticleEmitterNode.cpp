// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/ParticleEmitterNode.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/Components/SpatialComponent.h>
#include <AnKi/Scene/Components/ParticleEmitterComponent.h>

namespace anki {

/// Feedback component
class ParticleEmitterNode::MoveFeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(ParticleEmitterNode::MoveFeedbackComponent)

public:
	MoveFeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), true)
	{
	}

	Error update(SceneComponentUpdateInfo& info, Bool& updated)
	{
		updated = false; // Don't care about updates for this component

		MoveComponent& move = info.m_node->getFirstComponentOfType<MoveComponent>();
		if(move.getTimestamp() == info.m_node->getGlobalTimestamp())
		{
			static_cast<ParticleEmitterNode&>(*info.m_node).onMoveComponentUpdate(move);
		}

		return Error::kNone;
	}
};

ANKI_SCENE_COMPONENT_STATICS(ParticleEmitterNode::MoveFeedbackComponent, -1.0f)

/// Feedback component
class ParticleEmitterNode::ShapeFeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(ParticleEmitterNode::ShapeFeedbackComponent)

public:
	ShapeFeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), false // Not feedback because it's always being called
		)
	{
	}

	Error update(SceneComponentUpdateInfo& info, Bool& updated)
	{
		updated = false;
		static_cast<ParticleEmitterNode&>(*info.m_node).onShapeUpdate();
		return Error::kNone;
	}
};

ANKI_SCENE_COMPONENT_STATICS(ParticleEmitterNode::ShapeFeedbackComponent, -1.0f)

ParticleEmitterNode::ParticleEmitterNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
	// Components
	newComponent<MoveComponent>();

	newComponent<MoveFeedbackComponent>();

	newComponent<ParticleEmitterComponent>();

	newComponent<ShapeFeedbackComponent>();
	newComponent<SpatialComponent>();
}

ParticleEmitterNode::~ParticleEmitterNode()
{
}

void ParticleEmitterNode::onMoveComponentUpdate(MoveComponent& move)
{
	getFirstComponentOfType<SpatialComponent>().setSpatialOrigin(move.getWorldTransform().getOrigin().xyz());
}

void ParticleEmitterNode::onShapeUpdate()
{
	const ParticleEmitterComponent& pec = getFirstComponentOfType<ParticleEmitterComponent>();
	getFirstComponentOfType<SpatialComponent>().setAabbWorldSpace(pec.getAabbWorldSpace());
}

} // end namespace anki
