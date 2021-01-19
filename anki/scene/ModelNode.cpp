// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/ModelNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/DebugDrawer.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/SkinComponent.h>
#include <anki/scene/components/SpatialComponent.h>
#include <anki/scene/components/RenderComponent.h>
#include <anki/scene/components/ModelComponent.h>
#include <anki/resource/ModelResource.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/SkeletonResource.h>
#include <anki/physics/PhysicsWorld.h>

namespace anki
{

/// Feedback component.
class ModelNode::FeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(ModelNode::FeedbackComponent)

public:
	FeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), true)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;
		static_cast<ModelNode&>(node).feedbackUpdate();
		return Error::NONE;
	}
};

ANKI_SCENE_COMPONENT_STATICS(ModelNode::FeedbackComponent)

class ModelNode::RenderProxy
{
public:
	ModelNode* m_node = nullptr;
};

ModelNode::ModelNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
}

ModelNode::~ModelNode()
{
	m_renderProxies.destroy(getAllocator());
}

Error ModelNode::init()
{
	newComponent<ModelComponent>();
	newComponent<SkinComponent>();
	newComponent<MoveComponent>();
	newComponent<FeedbackComponent>();
	newComponent<SpatialComponent>();
	newComponent<RenderComponent>(); // One of many
	m_renderProxies.create(getAllocator(), 1);

	return Error::NONE;
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

	const U64 globTimestamp = getGlobalTimestamp();
	Bool updateSpatial = false;

	// Model update
	if(modelc.getTimestamp() == globTimestamp)
	{
		m_aabbLocal = modelc.getModelResource()->getBoundingVolume();
		updateSpatial = true;

		if(modelc.getModelResource()->getModelPatches().getSize() == countComponentsOfType<RenderComponent>())
		{
			// Easy, just re-init the render components
			initRenderComponents();
		}
		else
		{
			// Need to create more render components, deffer it
			ANKI_ASSERT(!"TODO");
		}
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
	}

	// Spatial update
	if(updateSpatial)
	{
		const Aabb aabbWorld = m_aabbLocal.getTransformed(movec.getWorldTransform());
		getFirstComponentOfType<SpatialComponent>().setAabbWorldSpace(aabbWorld);
	}
}

void ModelNode::initRenderComponents()
{
	const ModelComponent& modelc = getFirstComponentOfType<ModelComponent>();
	const ModelResourcePtr& model = modelc.getModelResource();

	ANKI_ASSERT(modelc.getModelResource()->getModelPatches().getSize() == countComponentsOfType<RenderComponent>());
	ANKI_ASSERT(modelc.getModelResource()->getModelPatches().getSize() == m_renderProxies.getSize());

	for(U32 patchIdx = 0; patchIdx < model->getModelPatches().getSize(); ++patchIdx)
	{
		RenderComponent& rc = getNthComponentOfType<RenderComponent>(patchIdx);
		rc.initRaster(
			[](RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData) {
				const RenderProxy& proxy = *static_cast<const RenderProxy*>(userData[0]);
				const U32 modelPatchIdx = U32(&proxy - &proxy.m_node->m_renderProxies[0]);
				proxy.m_node->draw(ctx, userData, modelPatchIdx);
			},
			&m_renderProxies[patchIdx], modelc.getRenderMergeKeys()[patchIdx]);

		rc.setFlagsFromMaterial(model->getModelPatches()[patchIdx].getMaterial());

		if(model->getModelPatches()[patchIdx].getSupportedRayTracingTypes() != RayTypeBit::NONE)
		{
			rc.initRayTracing(
				[](U32 lod, const void* userData, RayTracingInstanceQueueElement& el) {
					const RenderProxy& proxy = *static_cast<const RenderProxy*>(userData);
					const U32 modelPatchIdx = U32(&proxy - &proxy.m_node->m_renderProxies[0]);
					proxy.m_node->setupRayTracingInstanceQueueElement(lod, modelPatchIdx, el);
				},
				&m_renderProxies[patchIdx]);
		}

		m_renderProxies[patchIdx].m_node = this;
	}
}

void ModelNode::draw(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData, U32 modelPatchIdx) const
{
	const U32 instanceCount = userData.getSize();
	ANKI_ASSERT(instanceCount > 0 && instanceCount <= MAX_INSTANCES);
	ANKI_ASSERT(ctx.m_key.getInstanceCount() == instanceCount);

	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	if(ANKI_LIKELY(!ctx.m_debugDraw))
	{
		const ModelComponent& modelc = getFirstComponentOfType<ModelComponent>();
		const ModelPatch& patch = modelc.getModelResource()->getModelPatches()[modelPatchIdx];
		const SkinComponent& skinc = getFirstComponentOfType<SkinComponent>();

		// Transforms
		Array<Mat4, MAX_INSTANCES> trfs;
		Array<Mat4, MAX_INSTANCES> prevTrfs;
		const MoveComponent& movec = getFirstComponentOfType<MoveComponent>();
		trfs[0] = Mat4(movec.getWorldTransform());
		prevTrfs[0] = Mat4(movec.getPreviousWorldTransform());
		Bool moved = trfs[0] != prevTrfs[0];
		for(U32 i = 1; i < instanceCount; ++i)
		{
			const ModelNode& otherNode = *static_cast<const RenderProxy*>(userData[i])->m_node;

			const U32 otherNodeModelPatchIdx =
				U32(static_cast<const RenderProxy*>(userData[i]) - &otherNode.m_renderProxies[0]);
			(void)otherNodeModelPatchIdx;
			ANKI_ASSERT(otherNodeModelPatchIdx == modelPatchIdx);

			const MoveComponent& otherNodeMovec = otherNode.getFirstComponentOfType<MoveComponent>();
			trfs[i] = Mat4(otherNodeMovec.getWorldTransform());
			prevTrfs[i] = Mat4(otherNodeMovec.getPreviousWorldTransform());

			moved = moved || (trfs[i] != prevTrfs[i]);
		}

		ctx.m_key.setVelocity(moved && ctx.m_key.getPass() == Pass::GB);
		ctx.m_key.setSkinned(skinc.isEnabled());
		ModelRenderingInfo modelInf;
		patch.getRenderingInfo(ctx.m_key, modelInf);

		// Bones storage
		if(skinc.isEnabled())
		{
			const U32 boneCount = skinc.getBoneTransforms().getSize();
			StagingGpuMemoryToken token, tokenPrev;
			void* trfs = ctx.m_stagingGpuAllocator->allocateFrame(boneCount * sizeof(Mat4),
																  StagingGpuMemoryType::STORAGE, token);
			memcpy(trfs, &skinc.getBoneTransforms()[0], boneCount * sizeof(Mat4));

			trfs = ctx.m_stagingGpuAllocator->allocateFrame(boneCount * sizeof(Mat4), StagingGpuMemoryType::STORAGE,
															tokenPrev);
			memcpy(trfs, &skinc.getPreviousFrameBoneTransforms()[0], boneCount * sizeof(Mat4));

			ANKI_ASSERT(modelInf.m_boneTransformsBinding < MAX_U32);
			cmdb->bindStorageBuffer(patch.getMaterial()->getDescriptorSetIndex(), modelInf.m_boneTransformsBinding,
									token.m_buffer, token.m_offset, token.m_range);

			ANKI_ASSERT(modelInf.m_prevFrameBoneTransformsBinding < MAX_U32);
			cmdb->bindStorageBuffer(patch.getMaterial()->getDescriptorSetIndex(),
									modelInf.m_prevFrameBoneTransformsBinding, tokenPrev.m_buffer, tokenPrev.m_offset,
									tokenPrev.m_range);
		}

		// Program
		cmdb->bindShaderProgram(modelInf.m_program);

		// Uniforms
		RenderComponent::allocateAndSetupUniforms(
			modelc.getModelResource()->getModelPatches()[modelPatchIdx].getMaterial(), ctx,
			ConstWeakArray<Mat4>(&trfs[0], instanceCount), ConstWeakArray<Mat4>(&prevTrfs[0], instanceCount),
			*ctx.m_stagingGpuAllocator);

		// Set attributes
		for(U i = 0; i < modelInf.m_vertexAttributeCount; ++i)
		{
			const ModelVertexAttribute& attrib = modelInf.m_vertexAttributes[i];
			ANKI_ASSERT(attrib.m_format != Format::NONE);
			cmdb->setVertexAttribute(U32(attrib.m_location), attrib.m_bufferBinding, attrib.m_format,
									 attrib.m_relativeOffset);
		}

		// Set vertex buffers
		for(U32 i = 0; i < modelInf.m_vertexBufferBindingCount; ++i)
		{
			const ModelVertexBufferBinding& binding = modelInf.m_vertexBufferBindings[i];
			cmdb->bindVertexBuffer(i, binding.m_buffer, binding.m_offset, binding.m_stride, VertexStepRate::VERTEX);
		}

		// Index buffer
		cmdb->bindIndexBuffer(modelInf.m_indexBuffer, modelInf.m_indexBufferOffset, IndexType::U16);

		// Draw
		cmdb->drawElements(PrimitiveTopology::TRIANGLES, modelInf.m_indexCount, instanceCount, 0, 0, 0);
	}
	else
	{
		// Draw the bounding volumes

		Mat4* const mvps = ctx.m_frameAllocator.newArray<Mat4>(instanceCount);
		for(U32 i = 0; i < instanceCount; ++i)
		{
			const ModelNode& otherNode = *static_cast<const RenderProxy*>(userData[i])->m_node;
			const Aabb& box = otherNode.getFirstComponentOfType<SpatialComponent>().getAabbWorldSpace();
			const Vec4 tsl = (box.getMin() + box.getMax()) / 2.0f;
			const Vec3 scale = (box.getMax().xyz() - box.getMin().xyz()) / 2.0f;

			// Set non uniform scale. Add a margin to avoid flickering
			Mat3 nonUniScale = Mat3::getZero();
			constexpr F32 MARGIN = 1.02f;
			nonUniScale(0, 0) = scale.x() * MARGIN;
			nonUniScale(1, 1) = scale.y() * MARGIN;
			nonUniScale(2, 2) = scale.z() * MARGIN;

			mvps[i] = ctx.m_viewProjectionMatrix * Mat4(tsl.xyz1(), Mat3::getIdentity() * nonUniScale, 1.0f);
		}

		const Bool enableDepthTest = ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::DEPTH_TEST_ON);
		if(enableDepthTest)
		{
			cmdb->setDepthCompareOperation(CompareOperation::LESS);
		}
		else
		{
			cmdb->setDepthCompareOperation(CompareOperation::ALWAYS);
		}

		getSceneGraph().getDebugDrawer().drawCubes(
			ConstWeakArray<Mat4>(mvps, instanceCount), Vec4(1.0f, 0.0f, 1.0f, 1.0f), 2.0f,
			ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::DITHERED_DEPTH_TEST_ON), 2.0f,
			*ctx.m_stagingGpuAllocator, cmdb);

		ctx.m_frameAllocator.deleteArray(mvps, instanceCount);

		// Bones
		const SkinComponent& skinc = getFirstComponentOfType<SkinComponent>();
		if(skinc.isEnabled())
		{
			const SkinComponent& skinc = getComponentAt<SkinComponent>(0);
			SkeletonResourcePtr skeleton = skinc.getSkeleronResource();
			const U32 boneCount = skinc.getBoneTransforms().getSize();

			DynamicArrayAuto<Vec3> lines(ctx.m_frameAllocator);
			lines.resizeStorage(boneCount * 2);
			DynamicArrayAuto<Vec3> chidlessLines(ctx.m_frameAllocator);
			for(U32 i = 0; i < boneCount; ++i)
			{
				const Bone& bone = skeleton->getBones()[i];
				ANKI_ASSERT(bone.getIndex() == i);
				const Vec4 point(0.0f, 0.0f, 0.0f, 1.0f);
				const Bone* parent = bone.getParent();
				Mat4 m = (parent)
							 ? skinc.getBoneTransforms()[parent->getIndex()] * parent->getVertexTransform().getInverse()
							 : Mat4::getIdentity();
				const Vec3 a = (m * point).xyz();

				m = skinc.getBoneTransforms()[i] * bone.getVertexTransform().getInverse();
				const Vec3 b = (m * point).xyz();

				lines.emplaceBack(a);
				lines.emplaceBack(b);

				if(bone.getChildren().getSize() == 0)
				{
					// If there are not children try to draw something for that bone as well
					chidlessLines.emplaceBack(b);
					const F32 len = (b - a).getLength();
					const Vec3 c = b + (b - a).getNormalized() * len;
					chidlessLines.emplaceBack(c);
				}
			}

			const Mat4 mvp =
				ctx.m_viewProjectionMatrix * Mat4(getFirstComponentOfType<MoveComponent>().getWorldTransform());
			getSceneGraph().getDebugDrawer().drawLines(
				ConstWeakArray<Mat4>(&mvp, 1), Vec4(1.0f), 20.0f,
				ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::DITHERED_DEPTH_TEST_ON), lines,
				*ctx.m_stagingGpuAllocator, cmdb);

			getSceneGraph().getDebugDrawer().drawLines(
				ConstWeakArray<Mat4>(&mvp, 1), Vec4(0.7f, 0.7f, 0.7f, 1.0f), 5.0f,
				ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::DITHERED_DEPTH_TEST_ON), chidlessLines,
				*ctx.m_stagingGpuAllocator, cmdb);
		}

		// Restore state
		if(!enableDepthTest)
		{
			cmdb->setDepthCompareOperation(CompareOperation::LESS);
		}
	}
}

void ModelNode::setupRayTracingInstanceQueueElement(U32 lod, U32 modelPatchIdx,
													RayTracingInstanceQueueElement& el) const
{
	const ModelComponent& modelc = getFirstComponentOfType<ModelComponent>();
	const ModelPatch& patch = modelc.getModelResource()->getModelPatches()[modelPatchIdx];

	ModelRayTracingInfo info;
	patch.getRayTracingInfo(lod, info);

	memset(&el, 0, sizeof(el));

	// AS
	el.m_bottomLevelAccelerationStructure = info.m_bottomLevelAccelerationStructure.get();

	// Set the descriptor
	el.m_modelDescriptor = info.m_descriptor;
	const MoveComponent& movec = getFirstComponentOfType<MoveComponent>();
	const Mat3x4 worldTrf(movec.getWorldTransform());
	memcpy(&el.m_modelDescriptor.m_worldTransform, &worldTrf, sizeof(worldTrf));
	el.m_modelDescriptor.m_worldRotation = movec.getWorldTransform().getRotation().getRotationPart();

	// Handles
	for(RayType type : EnumIterable<RayType>())
	{
		if(!!(patch.getMaterial()->getSupportedRayTracingTypes() & RayTypeBit(1 << type)))
		{
			el.m_shaderGroupHandleIndices[type] = info.m_shaderGroupHandleIndices[type];
		}
		else
		{
			el.m_shaderGroupHandleIndices[type] = MAX_U32;
		}
	}

	// References
	ANKI_ASSERT(info.m_grObjectReferenceCount <= el.m_grObjects.getSize());
	el.m_grObjectCount = info.m_grObjectReferenceCount;
	for(U32 i = 0; i < info.m_grObjectReferenceCount; ++i)
	{
		el.m_grObjects[i] = info.m_grObjectReferences[i].get();
	}
}

} // end namespace anki
