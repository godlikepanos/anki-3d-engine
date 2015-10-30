// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/ModelNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/BodyComponent.h>
#include <anki/scene/Misc.h>
#include <anki/resource/Model.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/Skeleton.h>
#include <anki/physics/PhysicsWorld.h>

namespace anki {

//==============================================================================
// ModelPatchRenderComponent                                                   =
//==============================================================================

/// Render component implementation.
class ModelPatchRenderComponent: public RenderComponent
{
public:
	const ModelPatchNode& getNode() const
	{
		return static_cast<const ModelPatchNode&>(getSceneNode());
	}

	ModelPatchRenderComponent(ModelPatchNode* node)
		: RenderComponent(node, &node->m_modelPatch->getMaterial(),
			node->m_modelPatch->getModel().getUuid())
	{}

	ANKI_USE_RESULT Error buildRendering(
		RenderingBuildInfo& data) const override
	{
		return getNode().buildRendering(data);
	}

	void getRenderWorldTransform(Bool& hasTransform,
		Transform& trf) const override
	{
		hasTransform = true;
		const SceneNode* node = getNode().getParent();
		ANKI_ASSERT(node);
		trf = node->getComponent<MoveComponent>().getWorldTransform();
	}
};

//==============================================================================
// ModelPatchNode                                                              =
//==============================================================================

//==============================================================================
ModelPatchNode::ModelPatchNode(SceneGraph* scene)
	: SceneNode(scene)
{}

//==============================================================================
ModelPatchNode::~ModelPatchNode()
{}

//==============================================================================
Error ModelPatchNode::create(const CString& name,
	const ModelPatch* modelPatch)
{
	ANKI_ASSERT(modelPatch);
	ANKI_CHECK(SceneNode::create(name));

	m_modelPatch = modelPatch;

	// Spatial component
	SceneComponent* comp = getSceneAllocator().newInstance<SpatialComponent>(
		this, &m_obb);

	addComponent(comp, true);

	// Render component
	RenderComponent* rcomp =
		getSceneAllocator().newInstance<ModelPatchRenderComponent>(this);
	comp = rcomp;

	addComponent(comp, true);
	ANKI_CHECK(rcomp->create());

	return ErrorCode::NONE;
}

//==============================================================================
Error ModelPatchNode::buildRendering(RenderingBuildInfo& data) const
{
	// That will not work on multi-draw and instanced at the same time. Make
	// sure that there is no multi-draw anywhere
	ANKI_ASSERT(m_modelPatch->getSubMeshesCount() == 0);

	Array<U32, ANKI_GL_MAX_SUB_DRAWCALLS> indicesCountArray;
	Array<PtrSize, ANKI_GL_MAX_SUB_DRAWCALLS> indicesOffsetArray;
	U32 drawcallCount;

	PipelinePtr ppline;
	ResourceGroupPtr grResources;

	m_modelPatch->getRenderingDataSub(
		data.m_key,
		SArray<U8>(),
		grResources,
		ppline,
		indicesCountArray,
		indicesOffsetArray,
		drawcallCount);

	// Cannot accept multi-draw
	ANKI_ASSERT(drawcallCount == 1);

	// Set jobs
	data.m_cmdb->bindPipeline(ppline);
	data.m_cmdb->bindResourceGroup(grResources, 0, data.m_dynamicBufferInfo);

	// Drawcall
	U32 offset = indicesOffsetArray[0] / sizeof(U16);
	data.m_cmdb->drawElements(
		indicesCountArray[0],
		data.m_key.m_instanceCount,
		offset);

	return ErrorCode::NONE;
}

//==============================================================================
// ModelMoveFeedbackComponent                                                  =
//==============================================================================

/// Feedback component.
class ModelMoveFeedbackComponent: public SceneComponent
{
public:
	ModelMoveFeedbackComponent(SceneNode* node)
		: SceneComponent(SceneComponent::Type::NONE, node)
	{}

	ANKI_USE_RESULT Error update(
		SceneNode& node, F32, F32, Bool& updated)
	{
		updated = false;

		MoveComponent& move = node.getComponent<MoveComponent>();
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			ModelNode& mnode = static_cast<ModelNode&>(node);
			mnode.onMoveComponentUpdate(move);
		}

		return ErrorCode::NONE;
	}
};

//==============================================================================
// ModelNode                                                                   =
//==============================================================================

//==============================================================================
ModelNode::ModelNode(SceneGraph* scene)
	: SceneNode(scene)
{}

//==============================================================================
ModelNode::~ModelNode()
{
	m_modelPatches.destroy(getSceneAllocator());
}

//==============================================================================
Error ModelNode::create(const CString& name, const CString& modelFname)
{
	ANKI_CHECK(SceneNode::create(name));

	SceneComponent* comp;

	ANKI_CHECK(getResourceManager().loadResource(modelFname, m_model));

	m_modelPatches.create(
		getSceneAllocator(), m_model->getModelPatches().getSize(), nullptr);

	U count = 0;
	auto it = m_model->getModelPatches().getBegin();
	auto end = m_model->getModelPatches().getEnd();
	for(; it != end; it++)
	{
		ModelPatchNode* mpn;
		StringAuto nname(getFrameAllocator());
		nname.sprintf("%s_%d", &name[0], count);
		ANKI_CHECK(getSceneGraph().newSceneNode(nname.toCString(), mpn, *it));

		m_modelPatches[count++] = mpn;
		addChild(mpn);
	}

	// Move component
	comp = getSceneAllocator().newInstance<MoveComponent>(this);
	addComponent(comp, true);

	// Feedback component
	comp = getSceneAllocator().newInstance<ModelMoveFeedbackComponent>(this);
	addComponent(comp, true);

	return ErrorCode::NONE;
}

//==============================================================================
void ModelNode::onMoveComponentUpdate(MoveComponent& move)
{
	// Inform the children about the moves
	for(ModelPatchNode* child : m_modelPatches)
	{
		child->m_obb = child->m_modelPatch->getBoundingShape().getTransformed(
			move.getWorldTransform());

		SpatialComponent& sp = child->getComponent<SpatialComponent>();
		sp.markForUpdate();
		sp.setSpatialOrigin(move.getWorldTransform().getOrigin());
	}
}

} // end namespace anki
