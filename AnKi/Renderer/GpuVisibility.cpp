// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/GpuVisibility.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/Hzb.h>
#include <AnKi/Scene/RenderStateBucket.h>
#include <AnKi/Scene/ContiguousArrayAllocator.h>
#include <AnKi/Core/GpuMemory/GpuVisibleTransientMemoryPool.h>
#include <AnKi/Core/GpuMemory/RebarTransientMemoryPool.h>
#include <AnKi/Core/GpuMemory/GpuSceneBuffer.h>
#include <AnKi/Collision/Functions.h>
#include <AnKi/Shaders/Include/MiscRendererTypes.h>
#include <AnKi/Core/ConfigSet.h>

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
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/GpuVisibility.ankiprogbin", m_prog, m_grProg));

	return Error::kNone;
}

void GpuVisibility::populateRenderGraph(RenderingTechnique technique, const Mat4& viewProjectionMat, Vec3 cameraPosition, RenderTargetHandle hzbRt,
										RenderGraphDescription& rgraph)
{
	const U32 aabbCount = GpuSceneContiguousArrays::getSingleton().getElementCount(techniqueToArrayType(technique));
	const U32 bucketCount = RenderStateBucketContainer::getSingleton().getBucketCount(technique);

	// Allocate memory for the indirect commands
	const GpuVisibleTransientMemoryAllocation indirectArgs =
		GpuVisibleTransientMemoryPool::getSingleton().allocate(aabbCount * sizeof(DrawIndexedIndirectArgs));
	const GpuVisibleTransientMemoryAllocation instanceRateRenderables =
		GpuVisibleTransientMemoryPool::getSingleton().allocate(aabbCount * sizeof(GpuSceneRenderable));

	// Allocate and zero the MDI counts
	RebarAllocation mdiDrawCounts;
	U32* atomics = RebarTransientMemoryPool::getSingleton().allocateFrame<U32>(bucketCount, mdiDrawCounts);
	memset(atomics, 0, mdiDrawCounts.m_range);

	// Import buffers
	m_runCtx.m_instanceRateRenderables = rgraph.importBuffer(instanceRateRenderables.m_buffer, BufferUsageBit::kNone,
															 instanceRateRenderables.m_offset, instanceRateRenderables.m_size);
	m_runCtx.m_drawIndexedIndirectArgs =
		rgraph.importBuffer(indirectArgs.m_buffer, BufferUsageBit::kNone, indirectArgs.m_offset, indirectArgs.m_size);
	m_runCtx.m_mdiDrawCounts = rgraph.importBuffer(&RebarTransientMemoryPool::getSingleton().getBuffer(), BufferUsageBit::kNone,
												   mdiDrawCounts.m_offset, mdiDrawCounts.m_range);

	// Create the renderpass
	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("GPU occlusion");

	pass.newBufferDependency(getRenderer().getGpuSceneBufferHandle(), BufferUsageBit::kStorageComputeRead);
	pass.newTextureDependency(hzbRt, TextureUsageBit::kSampledCompute);
	pass.newBufferDependency(m_runCtx.m_instanceRateRenderables, BufferUsageBit::kStorageComputeWrite);
	pass.newBufferDependency(m_runCtx.m_drawIndexedIndirectArgs, BufferUsageBit::kStorageComputeWrite);
	pass.newBufferDependency(m_runCtx.m_mdiDrawCounts, BufferUsageBit::kStorageComputeWrite);

	pass.setWork([this, viewProjectionMat, cameraPosition, technique, hzbRt](RenderPassWorkContext& rpass) {
		CommandBuffer& cmdb = *rpass.m_commandBuffer;

		cmdb.bindShaderProgram(m_grProg.get());

		const GpuSceneContiguousArrayType type = techniqueToArrayType(technique);

		cmdb.bindStorageBuffer(0, 0, &GpuSceneBuffer::getSingleton().getBuffer(), GpuSceneContiguousArrays::getSingleton().getArrayBase(type),
							   GpuSceneContiguousArrays::getSingleton().getElementCount(type) * sizeof(GpuSceneRenderableAabb));

		cmdb.bindStorageBuffer(0, 1, &GpuSceneBuffer::getSingleton().getBuffer(),
							   GpuSceneContiguousArrays::getSingleton().getArrayBase(GpuSceneContiguousArrayType::kRenderables),
							   GpuSceneContiguousArrays::getSingleton().getElementCount(GpuSceneContiguousArrayType::kRenderables)
								   * sizeof(GpuSceneRenderable));

		cmdb.bindStorageBuffer(0, 2, &GpuSceneBuffer::getSingleton().getBuffer(), 0, kMaxPtrSize);

		rpass.bindColorTexture(0, 3, hzbRt);
		cmdb.bindSampler(0, 4, getRenderer().getSamplers().m_nearestNearestClamp.get());

		rpass.bindStorageBuffer(0, 5, m_runCtx.m_instanceRateRenderables);
		rpass.bindStorageBuffer(0, 6, m_runCtx.m_drawIndexedIndirectArgs);

		U32* offsets = allocateAndBindStorage<U32*>(sizeof(U32) * RenderStateBucketContainer::getSingleton().getBucketCount(technique), cmdb, 0, 7);
		U32 bucketCount = 0;
		U32 userCount = 0;
		RenderStateBucketContainer::getSingleton().iterateBuckets(technique, [&](const RenderStateInfo&, U32 userCount_) {
			offsets[bucketCount] = userCount;
			userCount += userCount_;
			++bucketCount;
		});
		ANKI_ASSERT(userCount == RenderStateBucketContainer::getSingleton().getBucketsItemCount(technique));

		rpass.bindStorageBuffer(0, 8, m_runCtx.m_mdiDrawCounts);

		GpuVisibilityUniforms* unis = allocateAndBindUniforms<GpuVisibilityUniforms*>(sizeof(GpuVisibilityUniforms), cmdb, 0, 9);

		Array<Plane, 6> planes;
		extractClipPlanes(viewProjectionMat, planes);
		for(U32 i = 0; i < 6; ++i)
		{
			unis->m_clipPlanes[i] = Vec4(planes[i].getNormal().xyz(), planes[i].getOffset());
		}

		const U32 aabbCount = GpuSceneContiguousArrays::getSingleton().getElementCount(type);
		unis->m_aabbCount = aabbCount;

		ANKI_ASSERT(kMaxLodCount == 3);
		unis->m_maxLodDistances[0] = ConfigSet::getSingleton().getLod0MaxDistance();
		unis->m_maxLodDistances[1] = ConfigSet::getSingleton().getLod1MaxDistance();
		unis->m_maxLodDistances[2] = kMaxF32;
		unis->m_maxLodDistances[3] = kMaxF32;

		unis->m_cameraOrigin = cameraPosition;
		unis->m_viewProjectionMat = viewProjectionMat;

		dispatchPPCompute(cmdb, 64, 1, aabbCount, 1);
	});
}

} // end namespace anki
