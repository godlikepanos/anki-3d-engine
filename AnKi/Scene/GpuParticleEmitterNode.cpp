// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/GpuParticleEmitterNode.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/Components/SpatialComponent.h>
#include <AnKi/Scene/Components/GenericGpuComputeJobComponent.h>
#include <AnKi/Scene/Components/GpuParticleEmitterComponent.h>

namespace anki {

/// Feedback component
class GpuParticleEmitterNode::MoveFeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(GpuParticleEmitterNode::MoveFeedbackComponent)

public:
	MoveFeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), true)
	{
	}

	Error update(SceneComponentUpdateInfo& info, Bool& updated)
	{
		updated = false;

		const MoveComponent& move = info.m_node->getFirstComponentOfType<MoveComponent>();
		if(move.getTimestamp() == info.m_node->getGlobalTimestamp())
		{
			GpuParticleEmitterNode& mnode = static_cast<GpuParticleEmitterNode&>(*info.m_node);
			mnode.onMoveComponentUpdate(move);
		}

		return Error::kNone;
	}
};

ANKI_SCENE_COMPONENT_STATICS(GpuParticleEmitterNode::MoveFeedbackComponent, -1.0f)

/// Feedback component
class GpuParticleEmitterNode::ShapeFeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(GpuParticleEmitterNode::ShapeFeedbackComponent)

public:
	ShapeFeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), true)
	{
	}

	Error update(SceneComponentUpdateInfo& info, Bool& updated)
	{
		updated = false;

		const GpuParticleEmitterComponent& pec = info.m_node->getFirstComponentOfType<GpuParticleEmitterComponent>();
		if(pec.getTimestamp() == info.m_node->getGlobalTimestamp())
		{
			GpuParticleEmitterNode& mnode = static_cast<GpuParticleEmitterNode&>(*info.m_node);
			mnode.onShapeUpdate(pec);
		}

		return Error::kNone;
	}
};

ANKI_SCENE_COMPONENT_STATICS(GpuParticleEmitterNode::ShapeFeedbackComponent, -1.0f)

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
}

GpuParticleEmitterNode::~GpuParticleEmitterNode()
{
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
