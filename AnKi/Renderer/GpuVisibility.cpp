// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/GpuVisibility.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Scene/RenderStateBucket.h>
#include <AnKi/Scene/ContiguousArrayAllocator.h>
#include <AnKi/Core/GpuMemory/GpuVisibleTransientMemoryPool.h>
#include <AnKi/Core/GpuMemory/RebarTransientMemoryPool.h>
#include <AnKi/Core/GpuMemory/GpuSceneBuffer.h>
#include <AnKi/Collision/Functions.h>
#include <AnKi/Shaders/Include/MiscRendererTypes.h>

namespace anki {

static GpuSceneContiguousArrayType techniqueToArrayType(RenderingTechnique technique)
{
	GpuSceneContiguousArrayType arrayType;
	switch(technique)
	{
	case RenderingTechnique::kGBuffer:
		arrayType = GpuSceneContiguousArrayType::kRenderableBoundingVolumesGBuffer;
		break;
	case RenderingTechnique::kDepth:
		arrayType = GpuSceneContiguousArrayType::kRenderableBoundingVolumesDepth;
		break;
	default:
		ANKI_ASSERT(0);
		arrayType = GpuSceneContiguousArrayType::kCount;
	}

	return arrayType;
}

Error GpuVisibility::init()
{
	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/GpuVisibility.ankiprogbin", m_prog));

	for(U32 i = 0; i < 2; ++i)
	{
		ShaderProgramResourceVariantInitInfo variantInit(m_prog);
		variantInit.addMutation("HZB_TEST", i);

		const ShaderProgramResourceVariant* variant;
		m_prog->getOrCreateVariant(variantInit, variant);

		m_grProgs[i].reset(&variant->getProgram());
	}

	return Error::kNone;
}

void GpuVisibility::populateRenderGraph(RenderingTechnique technique, const Mat4& viewProjectionMat, Vec3 lodReferencePoint,
										const Array<F32, kMaxLodCount - 1> lodDistances, const RenderTargetHandle* hzbRt,
										RenderGraphDescription& rgraph, GpuVisibilityOutput& out) const
{
	const U32 aabbCount = GpuSceneContiguousArrays::getSingleton().getElementCount(techniqueToArrayType(technique));
	const U32 bucketCount = RenderStateBucketContainer::getSingleton().getBucketCount(technique);

	// Allocate memory for the indirect commands
	const GpuVisibleTransientMemoryAllocation indirectArgs =
		GpuVisibleTransientMemoryPool::getSingleton().allocate(aabbCount * sizeof(DrawIndexedIndirectArgs));
	out.m_drawIndexedIndirectArgsBuffer = indirectArgs.m_buffer;
	out.m_drawIndexedIndirectArgsBufferOffset = indirectArgs.m_offset;
	out.m_drawIndexedIndirectArgsBufferRange = indirectArgs.m_size;

	const GpuVisibleTransientMemoryAllocation instanceRateRenderables =
		GpuVisibleTransientMemoryPool::getSingleton().allocate(aabbCount * sizeof(GpuSceneRenderable));
	out.m_instanceRateRenderablesBuffer = instanceRateRenderables.m_buffer;
	out.m_instanceRateRenderablesBufferOffset = instanceRateRenderables.m_offset;
	out.m_instanceRateRenderablesBufferRange = instanceRateRenderables.m_size;

	// Allocate and zero the MDI counts
	RebarAllocation mdiDrawCounts;
	U32* atomics = RebarTransientMemoryPool::getSingleton().allocateFrame<U32>(bucketCount, mdiDrawCounts);
	memset(atomics, 0, mdiDrawCounts.m_range);
	out.m_mdiDrawCountsBuffer = &RebarTransientMemoryPool::getSingleton().getBuffer();
	out.m_mdiDrawCountsBufferOffset = mdiDrawCounts.m_offset;
	out.m_mdiDrawCountsBufferRange = mdiDrawCounts.m_range;

	// Import buffers
	out.m_instanceRateRenderablesHandle = rgraph.importBuffer(instanceRateRenderables.m_buffer, BufferUsageBit::kNone,
															  instanceRateRenderables.m_offset, instanceRateRenderables.m_size);
	out.m_drawIndexedIndirectArgsHandle =
		rgraph.importBuffer(indirectArgs.m_buffer, BufferUsageBit::kNone, indirectArgs.m_offset, indirectArgs.m_size);
	out.m_mdiDrawCountsHandle = rgraph.importBuffer(&RebarTransientMemoryPool::getSingleton().getBuffer(), BufferUsageBit::kNone,
													mdiDrawCounts.m_offset, mdiDrawCounts.m_range);

	// Create the renderpass
	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("GPU occlusion");

	pass.newBufferDependency(getRenderer().getGpuSceneBufferHandle(), BufferUsageBit::kStorageComputeRead);
	pass.newBufferDependency(out.m_instanceRateRenderablesHandle, BufferUsageBit::kStorageComputeWrite);
	pass.newBufferDependency(out.m_drawIndexedIndirectArgsHandle, BufferUsageBit::kStorageComputeWrite);
	pass.newBufferDependency(out.m_mdiDrawCountsHandle, BufferUsageBit::kStorageComputeWrite);

	if(hzbRt)
	{
		pass.newTextureDependency(*hzbRt, TextureUsageBit::kSampledCompute);
	}

	const RenderTargetHandle hzbRtCopy =
		(hzbRt) ? *hzbRt : RenderTargetHandle(); // Can't pass to the lambda the hzbRt which is a pointer to who knows what

	pass.setWork([this, viewProjectionMat, lodReferencePoint, lodDistances, technique, hzbRtCopy,
				  drawIndexedIndirectArgsHandle = out.m_drawIndexedIndirectArgsHandle,
				  instanceRateRenderablesHandle = out.m_instanceRateRenderablesHandle,
				  mdiDrawCountsHandle = out.m_mdiDrawCountsHandle](RenderPassWorkContext& rpass) {
		CommandBuffer& cmdb = *rpass.m_commandBuffer;

		cmdb.bindShaderProgram(m_grProgs[hzbRtCopy.isValid()].get());

		const GpuSceneContiguousArrayType type = techniqueToArrayType(technique);

		cmdb.bindStorageBuffer(0, 0, &GpuSceneBuffer::getSingleton().getBuffer(), GpuSceneContiguousArrays::getSingleton().getArrayBase(type),
							   GpuSceneContiguousArrays::getSingleton().getElementCount(type) * sizeof(GpuSceneRenderableAabb));

		cmdb.bindStorageBuffer(0, 1, &GpuSceneBuffer::getSingleton().getBuffer(),
							   GpuSceneContiguousArrays::getSingleton().getArrayBase(GpuSceneContiguousArrayType::kRenderables),
							   GpuSceneContiguousArrays::getSingleton().getElementCount(GpuSceneContiguousArrayType::kRenderables)
								   * sizeof(GpuSceneRenderable));

		cmdb.bindStorageBuffer(0, 2, &GpuSceneBuffer::getSingleton().getBuffer(), 0, kMaxPtrSize);

		rpass.bindStorageBuffer(0, 3, instanceRateRenderablesHandle);
		rpass.bindStorageBuffer(0, 4, drawIndexedIndirectArgsHandle);

		U32* offsets = allocateAndBindStorage<U32*>(sizeof(U32) * RenderStateBucketContainer::getSingleton().getBucketCount(technique), cmdb, 0, 5);
		U32 bucketCount = 0;
		U32 userCount = 0;
		RenderStateBucketContainer::getSingleton().iterateBuckets(technique, [&](const RenderStateInfo&, U32 userCount_) {
			offsets[bucketCount] = userCount;
			userCount += userCount_;
			++bucketCount;
		});
		ANKI_ASSERT(userCount == RenderStateBucketContainer::getSingleton().getBucketsItemCount(technique));

		rpass.bindStorageBuffer(0, 6, mdiDrawCountsHandle);

		GpuVisibilityUniforms* unis = allocateAndBindUniforms<GpuVisibilityUniforms*>(sizeof(GpuVisibilityUniforms), cmdb, 0, 7);

		Array<Plane, 6> planes;
		extractClipPlanes(viewProjectionMat, planes);
		for(U32 i = 0; i < 6; ++i)
		{
			unis->m_clipPlanes[i] = Vec4(planes[i].getNormal().xyz(), planes[i].getOffset());
		}

		const U32 aabbCount = GpuSceneContiguousArrays::getSingleton().getElementCount(type);
		unis->m_aabbCount = aabbCount;

		ANKI_ASSERT(kMaxLodCount == 3);
		unis->m_maxLodDistances[0] = lodDistances[0];
		unis->m_maxLodDistances[1] = lodDistances[1];
		unis->m_maxLodDistances[2] = kMaxF32;
		unis->m_maxLodDistances[3] = kMaxF32;

		unis->m_lodReferencePoint = lodReferencePoint;
		unis->m_viewProjectionMat = viewProjectionMat;

		if(hzbRtCopy.isValid())
		{
			rpass.bindColorTexture(0, 8, hzbRtCopy);
			cmdb.bindSampler(0, 9, getRenderer().getSamplers().m_nearestNearestClamp.get());
		}

		dispatchPPCompute(cmdb, 64, 1, aabbCount, 1);
	});
}

} // end namespace anki
