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
#include <AnKi/Scene/Components/RenderComponent.h>
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

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override
	{
		updated = false;
		static_cast<ModelNode&>(*info.m_node).feedbackUpdate();
		return Error::kNone;
	}
};

ANKI_SCENE_COMPONENT_STATICS(ModelNode::FeedbackComponent)

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
	newComponent<ModelComponent>();
	newComponent<SkinComponent>();
	newComponent<MoveComponent>();
	newComponent<FeedbackComponent>();
	newComponent<SpatialComponent>();
	newComponent<RenderComponent>(); // One of many
	m_renderProxies.create(getMemoryPool(), 1);
}

ModelNode::~ModelNode()
{
	m_renderProxies.destroy(getMemoryPool());
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

		if(modelc.getModelResource()->getModelPatches().getSize() == countComponentsOfType<RenderComponent>())
		{
			// Easy, just re-init the render components
			initRenderComponents();
		}
		else
		{
			// Need to create more render components, can't do it at the moment, deffer it
			m_deferredRenderComponentUpdate = true;
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
		updateSpatial = true;
	}

	// Spatial update
	if(updateSpatial)
	{
		const Aabb aabbWorld = m_aabbLocal.getTransformed(movec.getWorldTransform());
		getFirstComponentOfType<SpatialComponent>().setAabbWorldSpace(aabbWorld);
	}
}

Error ModelNode::frameUpdate([[maybe_unused]] Second prevUpdateTime, [[maybe_unused]] Second crntTime)
{
	if(ANKI_LIKELY(!m_deferredRenderComponentUpdate))
	{
		return Error::kNone;
	}

	m_deferredRenderComponentUpdate = false;

	const ModelComponent& modelc = getFirstComponentOfType<ModelComponent>();
	const U32 modelPatchCount = modelc.getModelResource()->getModelPatches().getSize();
	const U32 renderComponentCount = countComponentsOfType<RenderComponent>();

	if(modelPatchCount > renderComponentCount)
	{
		const U32 diff = modelPatchCount - renderComponentCount;

		for(U32 i = 0; i < diff; ++i)
		{
			newComponent<RenderComponent>();
		}

		m_renderProxies.resize(getMemoryPool(), modelPatchCount);
	}
	else
	{
		ANKI_ASSERT(!"TODO");
	}

	ANKI_ASSERT(countComponentsOfType<RenderComponent>() == modelPatchCount);

	// Now you can init the render components
	initRenderComponents();

	return Error::kNone;
}

void ModelNode::initRenderComponents()
{
	const ModelComponent& modelc = getFirstComponentOfType<ModelComponent>();
	const ModelResourcePtr& model = modelc.getModelResource();

	ANKI_ASSERT(modelc.getModelResource()->getModelPatches().getSize() == countComponentsOfType<RenderComponent>());
	ANKI_ASSERT(modelc.getModelResource()->getModelPatches().getSize() == m_renderProxies.getSize());

	for(U32 patchIdx = 0; patchIdx < model->getModelPatches().getSize(); ++patchIdx)
	{
		const ModelPatch& modelPatch = model->getModelPatches()[patchIdx];
		RenderComponent& rc = getNthComponentOfType<RenderComponent>(patchIdx);
		rc.initRaster(
			[](RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData) {
				const RenderProxy& proxy = *static_cast<const RenderProxy*>(userData[0]);
				const U32 modelPatchIdx = U32(&proxy - &proxy.m_node->m_renderProxies[0]);
				proxy.m_node->draw(ctx, userData, modelPatchIdx);
			},
			&m_renderProxies[patchIdx], modelc.getRenderMergeKeys()[patchIdx]);

		rc.setFlagsFromMaterial(modelPatch.getMaterial());

		if(!!(modelPatch.getMaterial()->getRenderingTechniques() & RenderingTechniqueBit::kAllRt))
		{
			rc.initRayTracing(
				[](U32 lod, const void* userData, RayTracingInstanceQueueElement& el) {
					const RenderProxy& proxy = *static_cast<const RenderProxy*>(userData);
					const U32 modelPatchIdx = U32(&proxy - &proxy.m_node->m_renderProxies[0]);
					proxy.m_node->setupRayTracingInstanceQueueElement(lod, modelPatchIdx, el);
				},
				&m_renderProxies[patchIdx]);
		}

		// Init the proxy
		RenderProxy& proxy = m_renderProxies[patchIdx];
		proxy.m_node = this;

		const MeshResource& meshResource = *modelPatch.getMesh();
		proxy.m_compressedToModelTransform.setTranslationPart(meshResource.getPositionsTranslation().xyz1());
		proxy.m_compressedToModelTransform(0, 0) = meshResource.getPositionsScale();
		proxy.m_compressedToModelTransform(1, 1) = meshResource.getPositionsScale();
		proxy.m_compressedToModelTransform(2, 2) = meshResource.getPositionsScale();
	}
}

void ModelNode::draw(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData, U32 modelPatchIdx) const
{
	const U32 instanceCount = userData.getSize();
	ANKI_ASSERT(instanceCount > 0 && instanceCount <= kMaxInstanceCount);
	ANKI_ASSERT(ctx.m_key.getInstanceCount() == instanceCount);

	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	if(ANKI_LIKELY(!ctx.m_debugDraw))
	{
		const ModelComponent& modelc = getFirstComponentOfType<ModelComponent>();
		const ModelPatch& patch = modelc.getModelResource()->getModelPatches()[modelPatchIdx];
		const SkinComponent& skinc = getFirstComponentOfType<SkinComponent>();

		// Transforms
		auto computeTranform = [&](const Transform& trf) -> Mat3x4 {
			if(skinc.isEnabled())
			{
				return Mat3x4(trf);
			}
			else
			{
				// Bake the decompression in the model matrix
				const Mat4 m4 = Mat4(trf) * m_renderProxies[modelPatchIdx].m_compressedToModelTransform;
				const Mat3x4 out(m4);
				return out;
			}
		};
		Array<Mat3x4, kMaxInstanceCount> trfs;
		Array<Mat3x4, kMaxInstanceCount> prevTrfs;
		const MoveComponent& movec = getFirstComponentOfType<MoveComponent>();
		trfs[0] = computeTranform(movec.getWorldTransform());
		prevTrfs[0] = computeTranform(movec.getPreviousWorldTransform());
		Bool moved = trfs[0] != prevTrfs[0];
		for(U32 i = 1; i < instanceCount; ++i)
		{
			const ModelNode& otherNode = *static_cast<const RenderProxy*>(userData[i])->m_node;

			[[maybe_unused]] const U32 otherNodeModelPatchIdx =
				U32(static_cast<const RenderProxy*>(userData[i]) - &otherNode.m_renderProxies[0]);
			ANKI_ASSERT(otherNodeModelPatchIdx == modelPatchIdx);

			const MoveComponent& otherNodeMovec = otherNode.getFirstComponentOfType<MoveComponent>();
			trfs[i] = computeTranform(otherNodeMovec.getWorldTransform());
			prevTrfs[i] = computeTranform(otherNodeMovec.getPreviousWorldTransform());

			moved = moved || (trfs[i] != prevTrfs[i]);
		}

		ctx.m_key.setVelocity(moved && ctx.m_key.getRenderingTechnique() == RenderingTechnique::kGBuffer);
		ctx.m_key.setSkinned(skinc.isEnabled());
		ModelRenderingInfo modelInf;
		patch.getRenderingInfo(ctx.m_key, modelInf);

		// Bones storage
		if(skinc.isEnabled())
		{
			const U32 boneCount = skinc.getBoneTransforms().getSize();
			StagingGpuMemoryToken token, tokenPrev;
			void* trfs = ctx.m_stagingGpuAllocator->allocateFrame(boneCount * sizeof(Mat4),
																  StagingGpuMemoryType::kStorage, token);
			memcpy(trfs, &skinc.getBoneTransforms()[0], boneCount * sizeof(Mat4));

			trfs = ctx.m_stagingGpuAllocator->allocateFrame(boneCount * sizeof(Mat4), StagingGpuMemoryType::kStorage,
															tokenPrev);
			memcpy(trfs, &skinc.getPreviousFrameBoneTransforms()[0], boneCount * sizeof(Mat4));

			cmdb->bindStorageBuffer(kMaterialSetLocal, kMaterialBindingBoneTransforms, token.m_buffer, token.m_offset,
									token.m_range);

			cmdb->bindStorageBuffer(kMaterialSetLocal, kMaterialBindingPreviousBoneTransforms, tokenPrev.m_buffer,
									tokenPrev.m_offset, tokenPrev.m_range);
		}

		// Program
		cmdb->bindShaderProgram(modelInf.m_program);

		// Uniforms
		const Vec4 positionScaleAndTransform(
			m_renderProxies[modelPatchIdx].m_compressedToModelTransform(0, 0),
			m_renderProxies[modelPatchIdx].m_compressedToModelTransform.getTranslationPart().xyz());
		RenderComponent::allocateAndSetupUniforms(
			modelc.getModelResource()->getModelPatches()[modelPatchIdx].getMaterial(), ctx,
			ConstWeakArray<Mat3x4>(&trfs[0], instanceCount), ConstWeakArray<Mat3x4>(&prevTrfs[0], instanceCount),
			*ctx.m_stagingGpuAllocator, positionScaleAndTransform);

		// Bind attributes & vertex buffers
		for(VertexStreamId streamId :
			EnumIterable<VertexStreamId>(VertexStreamId::kMeshRelatedFirst, VertexStreamId::kMeshRelatedCount))
		{
			if(modelInf.m_vertexBufferOffsets[streamId] == kMaxPtrSize)
			{
				continue;
			}

			const U32 attribLocation = U32(streamId);
			const U32 bufferBinding = U32(streamId);
			const Format fmt = kMeshRelatedVertexStreamFormats[streamId];
			const U32 relativeOffset = 0;
			const U32 vertexStride = getFormatInfo(fmt).m_texelSize;

			cmdb->setVertexAttribute(attribLocation, bufferBinding, fmt, relativeOffset);

			cmdb->bindVertexBuffer(bufferBinding, getExternalSubsystems().m_unifiedGeometryMemPool->getBuffer(),
								   modelInf.m_vertexBufferOffsets[streamId], vertexStride, VertexStepRate::kVertex);
		}

		// Bind index buffer
		cmdb->bindIndexBuffer(getExternalSubsystems().m_unifiedGeometryMemPool->getBuffer(),
							  modelInf.m_indexBufferOffset, IndexType::kU16);

		// Draw
		cmdb->drawElements(PrimitiveTopology::kTriangles, modelInf.m_indexCount, instanceCount, modelInf.m_firstIndex,
						   0, 0);
	}
	else
	{
		// Draw the bounding volumes

		Mat4* const mvps = newArray<Mat4>(*ctx.m_framePool, instanceCount);
		for(U32 i = 0; i < instanceCount; ++i)
		{
			const ModelNode& otherNode = *static_cast<const RenderProxy*>(userData[i])->m_node;
			const Aabb& box = otherNode.getFirstComponentOfType<SpatialComponent>().getAabbWorldSpace();
			const Vec4 tsl = (box.getMin() + box.getMax()) / 2.0f;
			const Vec3 scale = (box.getMax().xyz() - box.getMin().xyz()) / 2.0f;

			// Set non uniform scale. Add a margin to avoid flickering
			Mat3 nonUniScale = Mat3::getZero();
			constexpr F32 kMargin = 1.02f;
			nonUniScale(0, 0) = scale.x() * kMargin;
			nonUniScale(1, 1) = scale.y() * kMargin;
			nonUniScale(2, 2) = scale.z() * kMargin;

			mvps[i] = ctx.m_viewProjectionMatrix * Mat4(tsl.xyz1(), Mat3::getIdentity() * nonUniScale, 1.0f);
		}

		const Bool enableDepthTest = ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::kDepthTestOn);
		if(enableDepthTest)
		{
			cmdb->setDepthCompareOperation(CompareOperation::kLess);
		}
		else
		{
			cmdb->setDepthCompareOperation(CompareOperation::kAlways);
		}

		getSceneGraph().getDebugDrawer().drawCubes(
			ConstWeakArray<Mat4>(mvps, instanceCount), Vec4(1.0f, 0.0f, 1.0f, 1.0f), 2.0f,
			ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::kDitheredDepthTestOn), 2.0f, *ctx.m_stagingGpuAllocator,
			cmdb);

		deleteArray(*ctx.m_framePool, mvps, instanceCount);

		// Bones
		const SkinComponent& skinc = getFirstComponentOfType<SkinComponent>();
		if(skinc.isEnabled())
		{
			const SkinComponent& skinc = getComponentAt<SkinComponent>(0);
			SkeletonResourcePtr skeleton = skinc.getSkeleronResource();
			const U32 boneCount = skinc.getBoneTransforms().getSize();

			DynamicArrayRaii<Vec3> lines(ctx.m_framePool);
			lines.resizeStorage(boneCount * 2);
			DynamicArrayRaii<Vec3> chidlessLines(ctx.m_framePool);
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
				ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::kDitheredDepthTestOn), lines,
				*ctx.m_stagingGpuAllocator, cmdb);

			getSceneGraph().getDebugDrawer().drawLines(
				ConstWeakArray<Mat4>(&mvp, 1), Vec4(0.7f, 0.7f, 0.7f, 1.0f), 5.0f,
				ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::kDitheredDepthTestOn), chidlessLines,
				*ctx.m_stagingGpuAllocator, cmdb);
		}

		// Restore state
		if(!enableDepthTest)
		{
			cmdb->setDepthCompareOperation(CompareOperation::kLess);
		}
	}
}

void ModelNode::setupRayTracingInstanceQueueElement(U32 lod, U32 modelPatchIdx,
													RayTracingInstanceQueueElement& el) const
{
	const ModelComponent& modelc = getFirstComponentOfType<ModelComponent>();
	const ModelPatch& patch = modelc.getModelResource()->getModelPatches()[modelPatchIdx];

	RenderingKey key(RenderingTechnique::kRtShadow, lod, 1, false, false);

	ModelRayTracingInfo info;
	patch.getRayTracingInfo(key, info);

	memset(&el, 0, sizeof(el));

	el.m_bottomLevelAccelerationStructure = info.m_bottomLevelAccelerationStructure.get();

	const MoveComponent& movec = getFirstComponentOfType<MoveComponent>();

	const Mat4 m4 = Mat4(movec.getWorldTransform()) * m_renderProxies[modelPatchIdx].m_compressedToModelTransform;
	el.m_transform = Mat3x4(m4);

	el.m_shaderGroupHandleIndex = info.m_shaderGroupHandleIndex;
}

} // end namespace anki
