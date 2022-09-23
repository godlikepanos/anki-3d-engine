// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/ParticleEmitterNode.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/Components/SpatialComponent.h>
#include <AnKi/Scene/Components/ParticleEmitterComponent.h>
#include <AnKi/Scene/Components/RenderComponent.h>

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

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override
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

ANKI_SCENE_COMPONENT_STATICS(ParticleEmitterNode::MoveFeedbackComponent)

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

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override
	{
		updated = false;
		static_cast<ParticleEmitterNode&>(*info.m_node).onShapeUpdate();
		return Error::kNone;
	}
};

ANKI_SCENE_COMPONENT_STATICS(ParticleEmitterNode::ShapeFeedbackComponent)

ParticleEmitterNode::ParticleEmitterNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
	// Components
	newComponent<MoveComponent>();

	newComponent<MoveFeedbackComponent>();

	ParticleEmitterComponent* particleEmitterc = newComponent<ParticleEmitterComponent>();

	newComponent<ShapeFeedbackComponent>();
	newComponent<SpatialComponent>();

	RenderComponent* rcomp = newComponent<RenderComponent>();
	rcomp->initRaster(ParticleEmitterComponent::drawCallback, particleEmitterc, 0); // No merging
}

ParticleEmitterNode::~ParticleEmitterNode()
{
}

void ParticleEmitterNode::onMoveComponentUpdate(MoveComponent& move)
{
	getFirstComponentOfType<ParticleEmitterComponent>().setWorldTransform(move.getWorldTransform());
	getFirstComponentOfType<SpatialComponent>().setSpatialOrigin(move.getWorldTransform().getOrigin().xyz());
}

void ParticleEmitterNode::onShapeUpdate()
{
	const ParticleEmitterComponent& pec = getFirstComponentOfType<ParticleEmitterComponent>();
	getFirstComponentOfType<SpatialComponent>().setAabbWorldSpace(pec.getAabbWorldSpace());
}

Error ParticleEmitterNode::frameUpdate([[maybe_unused]] Second prevUpdateTime, [[maybe_unused]] Second crntTime)
{
	const ParticleEmitterComponent& pec = getFirstComponentOfType<ParticleEmitterComponent>();
	if(pec.isEnabled())
	{
		RenderComponent& rc = getFirstComponentOfType<RenderComponent>();
		rc.setFlagsFromMaterial(pec.getParticleEmitterResource()->getMaterial());
	}

	return Error::kNone;
}

} // end namespace anki
