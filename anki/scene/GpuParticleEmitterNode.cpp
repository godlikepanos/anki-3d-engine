// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/GpuParticleEmitterNode.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/SpatialComponent.h>
#include <anki/scene/components/GenericGpuComputeJobComponent.h>
#include <anki/scene/components/RenderComponent.h>
#include <anki/scene/components/GpuParticleEmitterComponent.h>

namespace anki
{

/// Feedback component
class GpuParticleEmitterNode::MoveFeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(GpuParticleEmitterNode::MoveFeedbackComponent)

public:
	MoveFeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), true)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		const MoveComponent& move = node.getFirstComponentOfType<MoveComponent>();
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			GpuParticleEmitterNode& mnode = static_cast<GpuParticleEmitterNode&>(node);
			mnode.onMoveComponentUpdate(move);
		}

		return Error::NONE;
	}
};

ANKI_SCENE_COMPONENT_STATICS(GpuParticleEmitterNode::MoveFeedbackComponent)

/// Feedback component
class GpuParticleEmitterNode::ShapeFeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(GpuParticleEmitterNode::ShapeFeedbackComponent)

public:
	ShapeFeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), true)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		const GpuParticleEmitterComponent& pec = node.getFirstComponentOfType<GpuParticleEmitterComponent>();
		if(pec.getTimestamp() == node.getGlobalTimestamp())
		{
			GpuParticleEmitterNode& mnode = static_cast<GpuParticleEmitterNode&>(node);
			mnode.onShapeUpdate(pec);
		}

		return Error::NONE;
	}
};

ANKI_SCENE_COMPONENT_STATICS(GpuParticleEmitterNode::ShapeFeedbackComponent)

GpuParticleEmitterNode::GpuParticleEmitterNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
	// Create the components
	newComponent<MoveComponent>();
	newComponent<MoveFeedbackComponent>();
	GpuParticleEmitterComponent* pec = newComponent<GpuParticleEmitterComponent>();
	newComponent<ShapeFeedbackComponent>();
	newComponent<SpatialComponent>();

	GenericGpuComputeJobComponent* gpuComp = newComponent<GenericGpuComputeJobComponent>();
	gpuComp->setCallback(GpuParticleEmitterComponent::simulateCallback, pec);

	RenderComponent* rcomp = newComponent<RenderComponent>();
	const U64 noMerging = 0;
	rcomp->initRaster(GpuParticleEmitterComponent::drawCallback, pec, noMerging);
}

GpuParticleEmitterNode::~GpuParticleEmitterNode()
{
}

Error GpuParticleEmitterNode::frameUpdate(Second prevUpdateTime, Second crntTime)
{
	const GpuParticleEmitterComponent& pec = getFirstComponentOfType<GpuParticleEmitterComponent>();

	if(pec.isEnabled())
	{
		RenderComponent& rc = getFirstComponentOfType<RenderComponent>();
		rc.setFlagsFromMaterial(pec.getParticleEmitterResource()->getMaterial());
	}

	return Error::NONE;
}

void GpuParticleEmitterNode::onMoveComponentUpdate(const MoveComponent& movec)
{
	getFirstComponentOfType<SpatialComponent>().setSpatialOrigin(movec.getWorldTransform().getOrigin().xyz());
	getFirstComponentOfType<GpuParticleEmitterComponent>().setWorldTransform(movec.getWorldTransform());
}

void GpuParticleEmitterNode::onShapeUpdate(const GpuParticleEmitterComponent& pec)
{
	getFirstComponentOfType<SpatialComponent>().setAabbWorldSpace(pec.getBoundingVolumeWorldSpace());
}

} // end namespace anki
