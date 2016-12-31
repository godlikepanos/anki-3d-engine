// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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

namespace anki
{

/// Render component implementation.
class ModelPatchRenderComponent : public RenderComponent
{
public:
	const ModelPatchNode& getNode() const
	{
		return static_cast<const ModelPatchNode&>(getSceneNode());
	}

	ModelPatchRenderComponent(ModelPatchNode* node)
		: RenderComponent(node, &node->m_modelPatch->getMaterial())
	{
	}

	ANKI_USE_RESULT Error buildRendering(const RenderingBuildInfoIn& in, RenderingBuildInfoOut& out) const override
	{
		return getNode().buildRendering(in, out);
	}
};

ModelPatchNode::ModelPatchNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
}

ModelPatchNode::~ModelPatchNode()
{
}

Error ModelPatchNode::init(const ModelPatch* modelPatch)
{
	ANKI_ASSERT(modelPatch);

	m_modelPatch = modelPatch;

	// Spatial component
	SceneComponent* comp = getSceneAllocator().newInstance<SpatialComponent>(this, &m_obb);

	addComponent(comp, true);

	// Render component
	RenderComponent* rcomp = getSceneAllocator().newInstance<ModelPatchRenderComponent>(this);
	comp = rcomp;

	addComponent(comp, true);
	ANKI_CHECK(rcomp->init());

	return ErrorCode::NONE;
}

Error ModelPatchNode::buildRendering(const RenderingBuildInfoIn& in, RenderingBuildInfoOut& out) const
{
	// That will not work on multi-draw and instanced at the same time. Make sure that there is no multi-draw anywhere
	ANKI_ASSERT(m_modelPatch->getSubMeshesCount() == 0);

	// State
	ModelRenderingInfo modelInf;
	m_modelPatch->getRenderingDataSub(in.m_key, WeakArray<U8>(), modelInf);

	out.m_vertexBufferBindingCount = modelInf.m_vertexBufferBindingCount;
	for(U i = 0; i < modelInf.m_vertexBufferBindingCount; ++i)
	{
		static_cast<VertexBufferBinding&>(out.m_vertexBufferBindings[i]) = modelInf.m_vertexBufferBindings[i];
	}

	out.m_vertexAttributeCount = modelInf.m_vertexAttributeCount;
	for(U i = 0; i < modelInf.m_vertexAttributeCount; ++i)
	{
		out.m_vertexAttributes[i] = modelInf.m_vertexAttributes[i];
	}

	out.m_indexBuffer = modelInf.m_indexBuffer;

	out.m_program = modelInf.m_program;

	// Other
	ANKI_ASSERT(modelInf.m_drawcallCount == 1 && "Cannot accept multi-draw");
	out.m_drawcall.m_elements.m_count = modelInf.m_indicesCountArray[0];
	out.m_drawcall.m_elements.m_instanceCount = in.m_key.m_instanceCount;
	out.m_drawcall.m_elements.m_firstIndex = modelInf.m_indicesOffsetArray[0] / sizeof(U16);

	out.m_hasTransform = true;
	out.m_transform = Mat4(getParent()->getComponent<MoveComponent>().getWorldTransform());

	return ErrorCode::NONE;
}

/// Feedback component.
class ModelMoveFeedbackComponent : public SceneComponent
{
public:
	ModelMoveFeedbackComponent(SceneNode* node)
		: SceneComponent(SceneComponentType::NONE, node)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, F32, F32, Bool& updated)
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

ModelNode::ModelNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
}

ModelNode::~ModelNode()
{
	m_modelPatches.destroy(getSceneAllocator());
}

Error ModelNode::init(const CString& modelFname)
{
	SceneComponent* comp;

	ANKI_CHECK(getResourceManager().loadResource(modelFname, m_model));

	m_modelPatches.create(getSceneAllocator(), m_model->getModelPatches().getSize(), nullptr);

	U count = 0;
	auto it = m_model->getModelPatches().getBegin();
	auto end = m_model->getModelPatches().getEnd();
	for(; it != end; it++)
	{
		ModelPatchNode* mpn;
		StringAuto nname(getFrameAllocator());
		ANKI_CHECK(getSceneGraph().newSceneNode(CString(), mpn, *it));

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

void ModelNode::onMoveComponentUpdate(MoveComponent& move)
{
	// Inform the children about the moves
	for(ModelPatchNode* child : m_modelPatches)
	{
		child->m_obb = child->m_modelPatch->getBoundingShape().getTransformed(move.getWorldTransform());

		SpatialComponent& sp = child->getComponent<SpatialComponent>();
		sp.markForUpdate();
		sp.setSpatialOrigin(move.getWorldTransform().getOrigin());
	}
}

} // end namespace anki
