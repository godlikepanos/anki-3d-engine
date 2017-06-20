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
class ModelPatchNode::MRenderComponent : public RenderComponent
{
public:
	const ModelPatchNode& getNode() const
	{
		return static_cast<const ModelPatchNode&>(getSceneNode());
	}

	MRenderComponent(ModelPatchNode* node)
		: RenderComponent(node, node->m_modelPatch->getMaterial())
	{
	}

	void setupRenderableQueueElement(RenderableQueueElement& el) const override
	{
		getNode().setupRenderableQueueElement(el);
	}
};

ModelPatchNode::ModelPatchNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
}

ModelPatchNode::~ModelPatchNode()
{
}

Error ModelPatchNode::init(const ModelPatch* modelPatch, U idx, const ModelNode& parent)
{
	ANKI_ASSERT(modelPatch);

	m_modelPatch = modelPatch;

	// Spatial component
	newComponent<SpatialComponent>(this, &m_obb);

	// Render component
	newComponent<MRenderComponent>(this);

	// Merge key
	Array<U64, 2> toHash;
	toHash[0] = idx;
	toHash[1] = parent.m_model->getUuid();
	m_mergeKey = computeHash(&toHash[0], sizeof(toHash));

	return ErrorCode::NONE;
}

void ModelPatchNode::drawCallback(RenderQueueDrawContext& ctx, WeakArray<const void*> userData)
{
	ANKI_ASSERT(userData.getSize() > 0 && userData.getSize() <= MAX_INSTANCES);
	ANKI_ASSERT(ctx.m_key.m_instanceCount == userData.getSize());

	const ModelPatchNode& self = *static_cast<const ModelPatchNode*>(userData[0]);

	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	// That will not work on multi-draw and instanced at the same time. Make sure that there is no multi-draw anywhere
	ANKI_ASSERT(self.m_modelPatch->getSubMeshesCount() == 0);

	ModelRenderingInfo modelInf;
	self.m_modelPatch->getRenderingDataSub(ctx.m_key, WeakArray<U8>(), modelInf);

	// Program
	cmdb->bindShaderProgram(modelInf.m_program);

	// Set attributes
	for(U i = 0; i < modelInf.m_vertexAttributeCount; ++i)
	{
		const VertexAttributeInfo& attrib = modelInf.m_vertexAttributes[i];
		cmdb->setVertexAttribute(i, attrib.m_bufferBinding, attrib.m_format, attrib.m_relativeOffset);
	}

	// Set vertex buffers
	for(U i = 0; i < modelInf.m_vertexBufferBindingCount; ++i)
	{
		const VertexBufferBinding& binding = modelInf.m_vertexBufferBindings[i];
		cmdb->bindVertexBuffer(i, binding.m_buffer, binding.m_offset, binding.m_stride, VertexStepRate::VERTEX);
	}

	// Index buffer
	cmdb->bindIndexBuffer(modelInf.m_indexBuffer, 0, IndexType::U16);

	// Uniforms
	Array<Mat4, MAX_INSTANCES> trfs;
	trfs[0] = Mat4(self.getParent()->getComponentAt<MoveComponent>(0).getWorldTransform());
	for(U i = 1; i < userData.getSize(); ++i)
	{
		const ModelPatchNode& self2 = *static_cast<const ModelPatchNode*>(userData[i]);
		trfs[i] = Mat4(self2.getParent()->getComponentAt<MoveComponent>(0).getWorldTransform());
	}

	StagingGpuMemoryToken token;
	self.getComponentAt<RenderComponent>(1).allocateAndSetupUniforms(
		ctx, WeakArray<const Mat4>(&trfs[0], userData.getSize()), *ctx.m_stagingGpuAllocator, token);
	cmdb->bindUniformBuffer(0, 0, token.m_buffer, token.m_offset, token.m_range);

	// Draw
	cmdb->drawElements(PrimitiveTopology::TRIANGLES,
		modelInf.m_indicesCountArray[0],
		userData.getSize(),
		modelInf.m_indicesOffsetArray[0] / sizeof(U16),
		0,
		0);
}

/// Feedback component.
class ModelNode::MoveFeedbackComponent : public SceneComponent
{
public:
	MoveFeedbackComponent(SceneNode* node)
		: SceneComponent(SceneComponentType::NONE, node)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, F32, F32, Bool& updated)
	{
		updated = false;

		const MoveComponent& move = node.getComponentAt<MoveComponent>(0);
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			ModelNode& mnode = static_cast<ModelNode&>(node);
			mnode.onMoveComponentUpdate(move);
		}

		return ErrorCode::NONE;
	}
};

class ModelNode::MRenderComponent : public RenderComponent
{
public:
	MRenderComponent(ModelNode* node)
		: RenderComponent(node, node->m_model->getModelPatches()[0]->getMaterial())
	{
	}

	void setupRenderableQueueElement(RenderableQueueElement& el) const override
	{
		static_cast<const ModelNode&>(getSceneNode()).setupRenderableQueueElement(el);
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
	ANKI_CHECK(getResourceManager().loadResource(modelFname, m_model));

	if(m_model->getModelPatches().getSize() > 1)
	{
		// Multiple patches, create multiple nodes

		m_modelPatches.create(getSceneAllocator(), m_model->getModelPatches().getSize(), nullptr);

		U count = 0;
		auto it = m_model->getModelPatches().getBegin();
		auto end = m_model->getModelPatches().getEnd();
		for(; it != end; it++)
		{
			ModelPatchNode* mpn;
			StringAuto nname(getFrameAllocator());
			ANKI_CHECK(getSceneGraph().newSceneNode(CString(), mpn, *it, count, *this));

			m_modelPatches[count++] = mpn;
			addChild(mpn);
		}

		// Move component
		newComponent<MoveComponent>(this);

		// Feedback component
		newComponent<MoveFeedbackComponent>(this);
	}
	else
	{
		// Only one patch, don't need to create multiple nodes. Pack everything in this one.

		m_mergeKey = m_model->getUuid();

		newComponent<MoveComponent>(this);
		newComponent<MoveFeedbackComponent>(this);
		newComponent<SpatialComponent>(this, &m_obb);
		newComponent<MRenderComponent>(this);
	}

	return ErrorCode::NONE;
}

void ModelNode::onMoveComponentUpdate(const MoveComponent& move)
{
	if(!isSinglePatch())
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
	else
	{
		m_obb = m_model->getModelPatches()[0]->getBoundingShape().getTransformed(move.getWorldTransform());

		SpatialComponent& sp = getComponentAt<SpatialComponent>(2);
		sp.markForUpdate();
		sp.setSpatialOrigin(move.getWorldTransform().getOrigin());
	}
}

void ModelNode::drawCallback(RenderQueueDrawContext& ctx, WeakArray<const void*> userData)
{
	ANKI_ASSERT(userData.getSize() > 0 && userData.getSize() <= MAX_INSTANCES);
	ANKI_ASSERT(ctx.m_key.m_instanceCount == userData.getSize());

	const ModelNode& self = *static_cast<const ModelNode*>(userData[0]);
	ANKI_ASSERT(self.isSinglePatch());
	const ModelPatch* patch = self.m_model->getModelPatches()[0];

	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	// That will not work on multi-draw and instanced at the same time. Make sure that there is no multi-draw anywhere
	ANKI_ASSERT(patch->getSubMeshesCount() == 0);

	ModelRenderingInfo modelInf;
	patch->getRenderingDataSub(ctx.m_key, WeakArray<U8>(), modelInf);

	// Program
	cmdb->bindShaderProgram(modelInf.m_program);

	// Set attributes
	for(U i = 0; i < modelInf.m_vertexAttributeCount; ++i)
	{
		const VertexAttributeInfo& attrib = modelInf.m_vertexAttributes[i];
		cmdb->setVertexAttribute(i, attrib.m_bufferBinding, attrib.m_format, attrib.m_relativeOffset);
	}

	// Set vertex buffers
	for(U i = 0; i < modelInf.m_vertexBufferBindingCount; ++i)
	{
		const VertexBufferBinding& binding = modelInf.m_vertexBufferBindings[i];
		cmdb->bindVertexBuffer(i, binding.m_buffer, binding.m_offset, binding.m_stride, VertexStepRate::VERTEX);
	}

	// Index buffer
	cmdb->bindIndexBuffer(modelInf.m_indexBuffer, 0, IndexType::U16);

	// Uniforms
	Array<Mat4, MAX_INSTANCES> trfs;
	trfs[0] = Mat4(self.getComponentAt<MoveComponent>(0).getWorldTransform());
	for(U i = 1; i < userData.getSize(); ++i)
	{
		const ModelNode& self2 = *static_cast<const ModelNode*>(userData[i]);
		trfs[i] = Mat4(self2.getComponentAt<MoveComponent>(0).getWorldTransform());
	}

	StagingGpuMemoryToken token;
	self.getComponentAt<RenderComponent>(3).allocateAndSetupUniforms(
		ctx, WeakArray<const Mat4>(&trfs[0], userData.getSize()), *ctx.m_stagingGpuAllocator, token);
	cmdb->bindUniformBuffer(0, 0, token.m_buffer, token.m_offset, token.m_range);

	// Draw
	cmdb->drawElements(PrimitiveTopology::TRIANGLES,
		modelInf.m_indicesCountArray[0],
		userData.getSize(),
		modelInf.m_indicesOffsetArray[0] / sizeof(U16),
		0,
		0);
}

} // end namespace anki
