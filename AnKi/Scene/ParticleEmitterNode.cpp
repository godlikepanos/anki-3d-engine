// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
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

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false; // Don't care about updates for this component

		MoveComponent& move = node.getFirstComponentOfType<MoveComponent>();
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			static_cast<ParticleEmitterNode&>(node).onMoveComponentUpdate(move);
		}

		return Error::NONE;
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

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;
		static_cast<ParticleEmitterNode&>(node).onShapeUpdate();
		return Error::NONE;
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

Error ParticleEmitterNode::frameUpdate(Second prevUpdateTime, Second crntTime)
{
	const ParticleEmitterComponent& pec = getFirstComponentOfType<ParticleEmitterComponent>();
	if(pec.isEnabled())
	{
		RenderComponent& rc = getFirstComponentOfType<RenderComponent>();
		rc.setFlagsFromMaterial(pec.getParticleEmitterResource()->getMaterial());
	}

	return Error::NONE;
}

} // end namespace anki
