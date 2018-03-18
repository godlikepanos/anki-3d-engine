// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/ModelNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/components/BodyComponent.h>
#include <anki/scene/components/SkinComponent.h>
#include <anki/scene/Misc.h>
#include <anki/resource/ModelResource.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/SkeletonResource.h>
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

	return Error::NONE;
}

void ModelPatchNode::drawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData)
{
	ANKI_ASSERT(userData.getSize() > 0 && userData.getSize() <= MAX_INSTANCES);
	ANKI_ASSERT(ctx.m_key.m_instanceCount == userData.getSize());

	const ModelPatchNode& self = *static_cast<const ModelPatchNode*>(userData[0]);

	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	// That will not work on multi-draw and instanced at the same time. Make sure that there is no multi-draw anywhere
	ANKI_ASSERT(self.m_modelPatch->getSubMeshCount() == 1);

	ModelRenderingInfo modelInf;
	self.m_modelPatch->getRenderingDataSub(ctx.m_key, WeakArray<U8>(), modelInf);

	// Program
	cmdb->bindShaderProgram(modelInf.m_program);

	// Set attributes
	for(U i = 0; i < modelInf.m_vertexAttributeCount; ++i)
	{
		const VertexAttributeInfo& attrib = modelInf.m_vertexAttributes[i];
		cmdb->setVertexAttribute(
			U32(attrib.m_location), attrib.m_bufferBinding, attrib.m_format, attrib.m_relativeOffset);
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

	self.getComponentAt<RenderComponent>(1).allocateAndSetupUniforms(
		self.m_modelPatch->getMaterial()->getDescriptorSetIndex(),
		ctx,
		ConstWeakArray<Mat4>(&trfs[0], userData.getSize()),
		*ctx.m_stagingGpuAllocator);

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

	ANKI_USE_RESULT Error update(SceneNode& node, Second, Second, Bool& updated) override
	{
		updated = false;

		const MoveComponent& move = node.getComponent<MoveComponent>();
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			ModelNode& mnode = static_cast<ModelNode&>(node);
			mnode.onMoveComponentUpdate(move);
		}

		return Error::NONE;
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

		if(m_model->getSkeleton().isCreated())
		{
			newComponent<SkinComponent>(this, m_model->getSkeleton());
		}
		newComponent<MoveComponent>(this);
		newComponent<MoveFeedbackComponent>(this);
		newComponent<SpatialComponent>(this, &m_obb);
		newComponent<MRenderComponent>(this);
	}

	ANKI_CHECK(getResourceManager().loadResource("programs/SceneDebug.ankiprog", m_dbgProg));

	return Error::NONE;
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

		SpatialComponent& sp = getComponent<SpatialComponent>();
		sp.markForUpdate();
		sp.setSpatialOrigin(move.getWorldTransform().getOrigin());
	}
}

void ModelNode::drawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData)
{
	ANKI_ASSERT(userData.getSize() > 0 && userData.getSize() <= MAX_INSTANCES);
	ANKI_ASSERT(ctx.m_key.m_instanceCount == userData.getSize());

	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	const ModelNode& self = *static_cast<const ModelNode*>(userData[0]);
	ANKI_ASSERT(self.isSinglePatch());

	if(!ctx.m_debugDraw)
	{
		const ModelPatch* patch = self.m_model->getModelPatches()[0];

		// That will not work on multi-draw and instanced at the same time. Make sure that there is no multi-draw
		// anywhere
		ANKI_ASSERT(patch->getSubMeshCount() == 1);

		ModelRenderingInfo modelInf;
		patch->getRenderingDataSub(ctx.m_key, WeakArray<U8>(), modelInf);

		// Program
		cmdb->bindShaderProgram(modelInf.m_program);

		// Set attributes
		for(U i = 0; i < modelInf.m_vertexAttributeCount; ++i)
		{
			const VertexAttributeInfo& attrib = modelInf.m_vertexAttributes[i];
			ANKI_ASSERT(attrib.m_format != Format::NONE);
			cmdb->setVertexAttribute(
				U32(attrib.m_location), attrib.m_bufferBinding, attrib.m_format, attrib.m_relativeOffset);
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
		trfs[0] = Mat4(self.getComponent<MoveComponent>().getWorldTransform());
		for(U i = 1; i < userData.getSize(); ++i)
		{
			const ModelNode& self2 = *static_cast<const ModelNode*>(userData[i]);
			trfs[i] = Mat4(self2.getComponent<MoveComponent>().getWorldTransform());
		}

		self.getComponent<RenderComponent>().allocateAndSetupUniforms(patch->getMaterial()->getDescriptorSetIndex(),
			ctx,
			ConstWeakArray<Mat4>(&trfs[0], userData.getSize()),
			*ctx.m_stagingGpuAllocator);

		// Bones storage
		if(self.m_model->getSkeleton())
		{
			const SkinComponent& skinc = self.getComponentAt<SkinComponent>(0);
			StagingGpuMemoryToken token;
			void* trfs = ctx.m_stagingGpuAllocator->allocateFrame(
				skinc.getBoneTransforms().getSize() * sizeof(Mat4), StagingGpuMemoryType::STORAGE, token);
			memcpy(trfs, &skinc.getBoneTransforms()[0], skinc.getBoneTransforms().getSize() * sizeof(Mat4));

			cmdb->bindStorageBuffer(0, 0, token.m_buffer, token.m_offset, token.m_range);
		}

		// Draw
		cmdb->drawElements(PrimitiveTopology::TRIANGLES,
			modelInf.m_indicesCountArray[0],
			userData.getSize(),
			modelInf.m_indicesOffsetArray[0] / sizeof(U16),
			0,
			0);
	}
	else
	{
		// Draw the bounding volumes

		// Allocate staging memory
		StagingGpuMemoryToken vertToken;
		Vec3* verts = static_cast<Vec3*>(
			ctx.m_stagingGpuAllocator->allocateFrame(sizeof(Vec3) * 8, StagingGpuMemoryType::VERTEX, vertToken));

		const F32 SIZE = 1.0f;
		verts[0] = Vec3(SIZE, SIZE, SIZE); // front top right
		verts[1] = Vec3(-SIZE, SIZE, SIZE); // front top left
		verts[2] = Vec3(-SIZE, -SIZE, SIZE); // front bottom left
		verts[3] = Vec3(SIZE, -SIZE, SIZE); // front bottom right
		verts[4] = Vec3(SIZE, SIZE, -SIZE); // back top right
		verts[5] = Vec3(-SIZE, SIZE, -SIZE); // back top left
		verts[6] = Vec3(-SIZE, -SIZE, -SIZE); // back bottom left
		verts[7] = Vec3(SIZE, -SIZE, -SIZE); // back bottom right

		StagingGpuMemoryToken indicesToken;
		const U INDEX_COUNT = 12 * 2;
		U16* indices = static_cast<U16*>(ctx.m_stagingGpuAllocator->allocateFrame(
			sizeof(U16) * INDEX_COUNT, StagingGpuMemoryType::VERTEX, indicesToken));

		U c = 0;
		indices[c++] = 0;
		indices[c++] = 1;
		indices[c++] = 1;
		indices[c++] = 2;
		indices[c++] = 2;
		indices[c++] = 3;
		indices[c++] = 3;
		indices[c++] = 0;

		indices[c++] = 4;
		indices[c++] = 5;
		indices[c++] = 5;
		indices[c++] = 6;
		indices[c++] = 6;
		indices[c++] = 7;
		indices[c++] = 7;
		indices[c++] = 4;

		indices[c++] = 0;
		indices[c++] = 4;
		indices[c++] = 1;
		indices[c++] = 5;
		indices[c++] = 2;
		indices[c++] = 6;
		indices[c++] = 3;
		indices[c++] = 7;

		ANKI_ASSERT(c == INDEX_COUNT);

		// Set the uniforms
		StagingGpuMemoryToken unisToken;
		Mat4* mvps = static_cast<Mat4*>(ctx.m_stagingGpuAllocator->allocateFrame(
			sizeof(Mat4) * userData.getSize() + sizeof(Vec4), StagingGpuMemoryType::UNIFORM, unisToken));

		for(U i = 0; i < userData.getSize(); ++i)
		{
			const ModelNode& self2 = *static_cast<const ModelNode*>(userData[i]);

			Mat3 rot = self2.m_obb.getRotation().getRotationPart();
			const Vec4 tsl = self2.m_obb.getCenter().xyz1();
			const Vec3 scale = self2.m_obb.getExtend().xyz();

			// Set non uniform scale. Add a margin to avoid flickering
			const F32 MARGIN = 1.02;
			rot(0, 0) *= scale.x() * MARGIN;
			rot(1, 1) *= scale.y() * MARGIN;
			rot(2, 2) *= scale.z() * MARGIN;

			*mvps = ctx.m_viewProjectionMatrix * Mat4(tsl, rot, 1.0f);
			++mvps;
		}

		Vec4* color = reinterpret_cast<Vec4*>(mvps);
		*color = Vec4(1.0f, 0.0f, 1.0f, 1.0f);

		// Setup state
		ShaderProgramResourceMutationInitList<2> mutators(self.m_dbgProg);
		mutators.add("COLOR_TEXTURE", 0);
		mutators.add("DITHERED_DEPTH_TEST", ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::DITHERED_DEPTH_TEST_ON));
		ShaderProgramResourceConstantValueInitList<1> consts(self.m_dbgProg);
		consts.add("INSTANCE_COUNT", U32(userData.getSize()));
		const ShaderProgramResourceVariant* variant;
		self.m_dbgProg->getOrCreateVariant(mutators.get(), consts.get(), variant);
		cmdb->bindShaderProgram(variant->getProgram());

		const Bool enableDepthTest = ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::DEPTH_TEST_ON);
		if(enableDepthTest)
		{
			cmdb->setDepthCompareOperation(CompareOperation::LESS);
		}
		else
		{
			cmdb->setDepthCompareOperation(CompareOperation::ALWAYS);
		}

		cmdb->setVertexAttribute(0, 0, Format::R32G32B32_SFLOAT, 0);
		cmdb->bindVertexBuffer(0, vertToken.m_buffer, vertToken.m_offset, sizeof(Vec3));
		cmdb->bindIndexBuffer(indicesToken.m_buffer, indicesToken.m_offset, IndexType::U16);

		cmdb->bindUniformBuffer(1, 0, unisToken.m_buffer, unisToken.m_offset, unisToken.m_range);

		cmdb->drawElements(PrimitiveTopology::LINES, INDEX_COUNT, userData.getSize());

		// Restore state
		if(!enableDepthTest)
		{
			cmdb->setDepthCompareOperation(CompareOperation::LESS);
		}
	}
}

} // end namespace anki
