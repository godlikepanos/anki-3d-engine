// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/ModelNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/DebugDrawer.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/Components/SkinComponent.h>
#include <AnKi/Scene/Components/SpatialComponent.h>
#include <AnKi/Scene/Components/ModelComponent.h>
#include <AnKi/Resource/ModelResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/SkeletonResource.h>
#include <AnKi/Physics/PhysicsWorld.h>

namespace anki {

/// Feedback component.
class ModelNode::FeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(ModelNode::FeedbackComponent)

public:
	FeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), true)
	{
	}

	Error update(SceneComponentUpdateInfo& info, Bool& updated)
	{
		updated = false;
		static_cast<ModelNode&>(*info.m_node).feedbackUpdate();
		return Error::kNone;
	}
};

ANKI_SCENE_COMPONENT_STATICS(ModelNode::FeedbackComponent, -1.0f)

class ModelNode::RenderProxy
{
public:
	ModelNode* m_node = nullptr;

	/// Uncompresses the mesh positions to the local view. The scale should be uniform because it will be applied to
	/// normals and tangents and non-uniform data will cause problems.
	Mat4 m_compressedToModelTransform = Mat4::getIdentity();
};

ModelNode::ModelNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
	newComponent<SkinComponent>();
	newComponent<ModelComponent>();
	newComponent<MoveComponent>();
	newComponent<FeedbackComponent>();
	newComponent<SpatialComponent>();
}

ModelNode::~ModelNode()
{
}

void ModelNode::feedbackUpdate()
{
	const ModelComponent& modelc = getFirstComponentOfType<ModelComponent>();
	const SkinComponent& skinc = getFirstComponentOfType<SkinComponent>();
	const MoveComponent& movec = getFirstComponentOfType<MoveComponent>();

	if(!modelc.isEnabled())
	{
		// Disable everything
		ANKI_ASSERT(!"TODO");
		return;
	}

	const Timestamp globTimestamp = getGlobalTimestamp();
	Bool updateSpatial = false;

	// Model update
	if(modelc.getTimestamp() == globTimestamp)
	{
		m_aabbLocal = modelc.getModelResource()->getBoundingVolume();
		updateSpatial = true;
	}

	// Skin update
	if(skinc.isEnabled() && skinc.getTimestamp() == globTimestamp)
	{
		m_aabbLocal = skinc.getBoneBoundingVolumeLocalSpace();
		updateSpatial = true;
	}

	// Move update
	if(movec.getTimestamp() == globTimestamp)
	{
		getFirstComponentOfType<SpatialComponent>().setSpatialOrigin(movec.getWorldTransform().getOrigin().xyz());
		updateSpatial = true;
	}

	// Spatial update
	if(updateSpatial)
	{
		const Aabb aabbWorld = m_aabbLocal.getTransformed(movec.getWorldTransform());
		getFirstComponentOfType<SpatialComponent>().setAabbWorldSpace(aabbWorld);
	}
}

} // end namespace anki
