// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/GpuVisibility.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/HiZ.h>
#include <AnKi/Scene/RenderStateBucket.h>
#include <AnKi/Scene/ContiguousArrayAllocator.h>
#include <AnKi/Core/GpuMemory/GpuVisibleTransientMemoryPool.h>
#include <AnKi/Core/GpuMemory/RebarTransientMemoryPool.h>
#include <AnKi/Core/GpuMemory/GpuSceneBuffer.h>
#include <AnKi/Collision/Functions.h>

namespace anki {

Error GpuVisibility::init()
{
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/GpuVisibility.ankiprogbin", m_prog, m_grProg));

	return Error::kNone;
}

void GpuVisibility::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	U32 activeBucketCount = 0;
	U32 aabbCount = 0;
	RenderStateBucketContainer::getSingleton().iterateBuckets(RenderingTechnique::kGBuffer,
															  [&](const RenderStateInfo&, U32 userCount) {
																  ++activeBucketCount;
																  aabbCount += userCount;
															  });

	ANKI_ASSERT(aabbCount
				== AllGpuSceneContiguousArrays::getSingleton().getElementCount(
					GpuSceneContiguousArrayType::kRenderableBoundingVolumesGBuffer));
	aabbCount = AllGpuSceneContiguousArrays::getSingleton().getElementCount(
		GpuSceneContiguousArrayType::kRenderableBoundingVolumesGBuffer);

	// Allocate memory for the indirect commands
	const GpuVisibleTransientMemoryAllocation indirectCalls =
		GpuVisibleTransientMemoryPool::getSingleton().allocate(aabbCount * sizeof(DrawIndexedIndirectInfo));
	const GpuVisibleTransientMemoryAllocation instanceRateRenderables =
		GpuVisibleTransientMemoryPool::getSingleton().allocate(aabbCount * sizeof(GpuSceneRenderable));

	// Allocate and fill the offsets to the atomic counters for each bucket
	RebarAllocation atomicsAlloc;
	U32* atomics = static_cast<U32*>(
		RebarTransientMemoryPool::getSingleton().allocateFrame(activeBucketCount * sizeof(U32), atomicsAlloc));
	U32 count = 0;
	activeBucketCount = 0;
	RenderStateBucketContainer::getSingleton().iterateBuckets(RenderingTechnique::kGBuffer,
															  [&](const RenderStateInfo&, U32 userCount) {
																  atomics[activeBucketCount] = count;
																  count += userCount;
																  ++activeBucketCount;
															  });

	// Import buffers
	m_runCtx.m_instanceRateRenderables =
		rgraph.importBuffer(BufferPtr(instanceRateRenderables.m_buffer), BufferUsageBit::kNone,
							instanceRateRenderables.m_offset, instanceRateRenderables.m_size);
	m_runCtx.m_drawIndexedIndirects = rgraph.importBuffer(BufferPtr(indirectCalls.m_buffer), BufferUsageBit::kNone,
														  indirectCalls.m_offset, indirectCalls.m_size);
	m_runCtx.m_drawIndirectOffsets =
		rgraph.importBuffer(RebarTransientMemoryPool::getSingleton().getBuffer(), BufferUsageBit::kNone,
							atomicsAlloc.m_offset, atomicsAlloc.m_range);

	// Create the renderpass
	constexpr BufferUsageBit bufferUsage = BufferUsageBit::kStorageComputeRead;
	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("GPU occlusion GBuffer");

	pass.newBufferDependency(getRenderer().getGpuSceneBufferHandle(), bufferUsage);
	pass.newTextureDependency(getRenderer().getHiZ().getHiZRt(), TextureUsageBit::kSampledCompute);
	pass.newBufferDependency(m_runCtx.m_instanceRateRenderables, bufferUsage);
	pass.newBufferDependency(m_runCtx.m_drawIndexedIndirects, bufferUsage);
	pass.newBufferDependency(m_runCtx.m_drawIndirectOffsets, bufferUsage);

	pass.setWork([this, &ctx](RenderPassWorkContext& rpass) {
		CommandBufferPtr& cmdb = rpass.m_commandBuffer;

		cmdb->bindShaderProgram(m_grProg);

		cmdb->bindStorageBuffer(0, 0, GpuSceneBuffer::getSingleton().getBuffer(),
								AllGpuSceneContiguousArrays::getSingleton().getArrayBase(
									GpuSceneContiguousArrayType::kRenderableBoundingVolumesGBuffer),
								AllGpuSceneContiguousArrays::getSingleton().getElementCount(
									GpuSceneContiguousArrayType::kRenderableBoundingVolumesGBuffer)
									* sizeof(GpuSceneRenderableAabb));

		cmdb->bindStorageBuffer(
			0, 1, GpuSceneBuffer::getSingleton().getBuffer(),
			AllGpuSceneContiguousArrays::getSingleton().getArrayBase(GpuSceneContiguousArrayType::kRenderables),
			AllGpuSceneContiguousArrays::getSingleton().getElementCount(GpuSceneContiguousArrayType::kRenderables)
				* sizeof(GpuSceneRenderable));

		cmdb->bindStorageBuffer(0, 2, GpuSceneBuffer::getSingleton().getBuffer(), 0, kMaxPtrSize);

		rpass.bindColorTexture(0, 3, getRenderer().getHiZ().getHiZRt());

		rpass.bindStorageBuffer(0, 4, m_runCtx.m_instanceRateRenderables);
		rpass.bindStorageBuffer(0, 5, m_runCtx.m_drawIndexedIndirects);
		rpass.bindStorageBuffer(0, 6, m_runCtx.m_drawIndirectOffsets);

		struct Uniforms
		{
			Vec4 m_clipPlanes[6u];

			UVec3 m_padding;
			U32 m_aabbCount;
		} unis;

		Array<Plane, 6> planes;
		extractClipPlanes(ctx.m_matrices.m_viewProjection, planes);
		for(U32 i = 0; i < 6; ++i)
		{
			unis.m_clipPlanes[i] = Vec4(planes[i].getNormal().xyz(), planes[i].getOffset());
		}

		unis.m_aabbCount = AllGpuSceneContiguousArrays::getSingleton().getElementCount(
			GpuSceneContiguousArrayType::kRenderableBoundingVolumesGBuffer);
		cmdb->setPushConstants(&unis, sizeof(unis));

		dispatchPPCompute(cmdb, 64, 1, 1, unis.m_aabbCount, 1, 1);
	});
}

} // end namespace anki
