// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Utils/GpuVisibility.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Scene/RenderStateBucket.h>
#include <AnKi/Scene/ContiguousArrayAllocator.h>
#include <AnKi/Core/GpuMemory/GpuVisibleTransientMemoryPool.h>
#include <AnKi/Core/GpuMemory/RebarTransientMemoryPool.h>
#include <AnKi/Core/GpuMemory/GpuSceneBuffer.h>
#include <AnKi/Collision/Functions.h>
#include <AnKi/Shaders/Include/GpuVisibilityTypes.h>
#include <AnKi/Core/StatsSet.h>

namespace anki {

static StatCounter g_visibleObjects(StatCategory::kMisc, "Visible objects", StatFlag::kZeroEveryFrame);
static StatCounter g_testedObjects(StatCategory::kMisc, "Visbility tested objects", StatFlag::kZeroEveryFrame);

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
		variantInit.addMutation("STATS", ANKI_STATS_ENABLED);

		const ShaderProgramResourceVariant* variant;
		m_prog->getOrCreateVariant(variantInit, variant);

		m_grProgs[i].reset(&variant->getProgram());
	}

#if ANKI_STATS_ENABLED
	for(GpuReadbackMemoryAllocation& alloc : m_readbackMemory)
	{
		alloc = GpuReadbackMemoryPool::getSingleton().allocate(sizeof(U32));
	}
#endif

	return Error::kNone;
}

void GpuVisibility::populateRenderGraph(CString passesName, RenderingTechnique technique, const Mat4& viewProjectionMat, Vec3 lodReferencePoint,
										const Array<F32, kMaxLodCount - 1> lodDistances, const RenderTargetHandle* hzbRt,
										RenderGraphDescription& rgraph, GpuVisibilityOutput& out)
{
	const U32 aabbCount = GpuSceneContiguousArrays::getSingleton().getElementCount(techniqueToArrayType(technique));
	const U32 bucketCount = RenderStateBucketContainer::getSingleton().getBucketCount(technique);

#if ANKI_STATS_ENABLED
	Bool firstCallInTheFrame = false;
	if(m_lastFrameIdx != getRenderer().getFrameCount())
	{
		firstCallInTheFrame = true;
		m_lastFrameIdx = getRenderer().getFrameCount();
	}

	const GpuReadbackMemoryAllocation& readAlloc = m_readbackMemory[(m_lastFrameIdx + 1) % m_readbackMemory.getSize()];
	const GpuReadbackMemoryAllocation& writeAlloc = m_readbackMemory[m_lastFrameIdx % m_readbackMemory.getSize()];

	Buffer* clearStatsBuffer = &readAlloc.getBuffer();
	const PtrSize clearStatsBufferOffset = readAlloc.getOffset();
	Buffer* writeStatsBuffer = &writeAlloc.getBuffer();
	const PtrSize writeStatsBufferOffset = writeAlloc.getOffset();

	if(firstCallInTheFrame)
	{
		U32 visibleCount;
		memcpy(&visibleCount, readAlloc.getMappedMemory(), sizeof(visibleCount));

		g_visibleObjects.set(visibleCount);
	}

	g_testedObjects.increment(aabbCount);
#endif

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
	out.m_mdiDrawCountsHandle = rgraph.importBuffer(&RebarTransientMemoryPool::getSingleton().getBuffer(), BufferUsageBit::kNone,
													mdiDrawCounts.m_offset, mdiDrawCounts.m_range);

	// Create the renderpass
	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass(passesName);

	pass.newBufferDependency(getRenderer().getGpuSceneBufferHandle(), BufferUsageBit::kStorageComputeRead);
	pass.newBufferDependency(out.m_mdiDrawCountsHandle, BufferUsageBit::kStorageComputeWrite);

	if(hzbRt)
	{
		pass.newTextureDependency(*hzbRt, TextureUsageBit::kSampledCompute);
	}

	const RenderTargetHandle hzbRtCopy =
		(hzbRt) ? *hzbRt : RenderTargetHandle(); // Can't pass to the lambda the hzbRt which is a pointer to who knows what

	pass.setWork([this, viewProjectionMat, lodReferencePoint, lodDistances, technique, hzbRtCopy, mdiDrawCountsHandle = out.m_mdiDrawCountsHandle,
				  instanceRateRenderables, indirectArgs
#if ANKI_STATS_ENABLED
				  ,
				  clearStatsBuffer, clearStatsBufferOffset, writeStatsBuffer, writeStatsBufferOffset
#endif
	](RenderPassWorkContext& rpass) {
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

		cmdb.bindStorageBuffer(0, 3, instanceRateRenderables.m_buffer, instanceRateRenderables.m_offset, instanceRateRenderables.m_size);
		cmdb.bindStorageBuffer(0, 4, indirectArgs.m_buffer, indirectArgs.m_offset, indirectArgs.m_size);

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

#if ANKI_STATS_ENABLED
		cmdb.bindStorageBuffer(0, 10, writeStatsBuffer, writeStatsBufferOffset, sizeof(U32));
		cmdb.bindStorageBuffer(0, 11, clearStatsBuffer, clearStatsBufferOffset, sizeof(U32));
#endif

		dispatchPPCompute(cmdb, 64, 1, aabbCount, 1);
	});
}

Error GpuVisibilityNonRenderables::init()
{
	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/GpuVisibilityNonRenderables.ankiprogbin", m_prog));

	for(U32 hzb = 0; hzb < 2; ++hzb)
	{
		for(GpuSceneNonRenderableObjectType type : EnumIterable<GpuSceneNonRenderableObjectType>())
		{
			for(U32 cpuFeedback = 0; cpuFeedback < 2; ++cpuFeedback)
			{
				ShaderProgramResourceVariantInitInfo variantInit(m_prog);
				variantInit.addMutation("HZB_TEST", hzb);
				variantInit.addMutation("OBJECT_TYPE", U32(type));
				variantInit.addMutation("CPU_FEEDBACK", cpuFeedback);

				const ShaderProgramResourceVariant* variant;
				m_prog->getOrCreateVariant(variantInit, variant);

				if(variant)
				{
					m_grProgs[hzb][type][cpuFeedback].reset(&variant->getProgram());
				}
				else
				{
					m_grProgs[hzb][type][cpuFeedback].reset(nullptr);
				}
			}
		}
	}

	{
		CommandBufferInitInfo cmdbInit("TmpClear");
		cmdbInit.m_flags |= CommandBufferFlag::kSmallBatch;
		CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cmdbInit);

		for(U32 i = 0; i < kMaxFeedbackRequestsPerFrame; ++i)
		{
			BufferInitInfo buffInit("GpuVisibilityNonRenderablesFeedbackCounters");
			buffInit.m_size = 2 * sizeof(U32);
			buffInit.m_usage = BufferUsageBit::kStorageComputeWrite | BufferUsageBit::kTransferDestination;

			m_counterBuffers[i] = GrManager::getSingleton().newBuffer(buffInit);

			cmdb->fillBuffer(m_counterBuffers[i].get(), 0, kMaxPtrSize, 0);
		}

		cmdb->flush();
		GrManager::getSingleton().finish();
	}

	return Error::kNone;
}

void GpuVisibilityNonRenderables::populateRenderGraph(GpuVisibilityNonRenderablesInput& in, GpuVisibilityNonRenderablesOutput& out)
{
	const GpuSceneContiguousArrayType arrayType = gpuSceneNonRenderableObjectTypeToGpuSceneContiguousArrayType(in.m_objectType);
	const U32 objCount = GpuSceneContiguousArrays::getSingleton().getElementCount(arrayType);

	if(objCount == 0)
	{
		return;
	}

	if(in.m_cpuFeedback.m_buffer)
	{
		ANKI_ASSERT(in.m_cpuFeedback.m_bufferRange == sizeof(U32) * (objCount + 1));
	}

	// Find the counter buffer required for feedback
	U32 counterBufferIdx = kMaxU32;
	if(in.m_cpuFeedback.m_buffer)
	{
		if(m_lastFrameIdx != getRenderer().getFrameCount())
		{
			m_lastFrameIdx = getRenderer().getFrameCount();
			m_feedbackRequestCountThisFrame = 0;
		}

		counterBufferIdx = m_feedbackRequestCountThisFrame++;
		m_counterIdx[counterBufferIdx] = (m_counterIdx[counterBufferIdx] + 1) & 1;
	}

	// Allocate memory for the result
	RebarAllocation visibleIndicesAlloc;
	U32* indices = RebarTransientMemoryPool::getSingleton().allocateFrame<U32>(objCount, visibleIndicesAlloc);
	indices[0] = 0;

	out.m_visibleIndicesBuffer = &RebarTransientMemoryPool::getSingleton().getBuffer();
	out.m_visibleIndicesBufferOffset = visibleIndicesAlloc.m_offset;
	out.m_visibleIndicesBufferRange = visibleIndicesAlloc.m_range;

	// Import buffers
	RenderGraphDescription& rgraph = *in.m_rgraph;
	out.m_bufferHandle =
		rgraph.importBuffer(out.m_visibleIndicesBuffer, BufferUsageBit::kNone, out.m_visibleIndicesBufferOffset, out.m_visibleIndicesBufferRange);

	// Create the renderpass
	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass(in.m_passesName);

	pass.newBufferDependency(getRenderer().getGpuSceneBufferHandle(), BufferUsageBit::kStorageComputeRead);
	pass.newBufferDependency(out.m_bufferHandle, BufferUsageBit::kStorageComputeWrite);

	if(in.m_hzbRt)
	{
		pass.newTextureDependency(*in.m_hzbRt, TextureUsageBit::kSampledCompute);
	}

	pass.setWork([this, objType = in.m_objectType, feedbackBuffer = in.m_cpuFeedback.m_buffer, feedbackBufferOffset = in.m_cpuFeedback.m_bufferOffset,
				  feedbackBufferRange = in.m_cpuFeedback.m_bufferRange, viewProjectionMat = in.m_viewProjectionMat,
				  visibleIndicesBuffHandle = out.m_bufferHandle, counterBufferIdx,
				  counterIdx = m_counterIdx[counterBufferIdx]](RenderPassWorkContext& rgraph) {
		CommandBuffer& cmdb = *rgraph.m_commandBuffer;
		const GpuSceneContiguousArrayType arrayType = gpuSceneNonRenderableObjectTypeToGpuSceneContiguousArrayType(objType);
		const U32 objCount = GpuSceneContiguousArrays::getSingleton().getElementCount(arrayType);
		const GpuSceneContiguousArrays& cArrays = GpuSceneContiguousArrays::getSingleton();
		const Bool needsFeedback = feedbackBuffer != nullptr;

		cmdb.bindShaderProgram(m_grProgs[0][objType][needsFeedback].get());

		cmdb.bindStorageBuffer(0, 0, &GpuSceneBuffer::getSingleton().getBuffer(), cArrays.getArrayBase(arrayType),
							   cArrays.getElementCount(GpuSceneContiguousArrayType::kRenderables),
							   cArrays.getElementSize(arrayType) * cArrays.getElementCount(arrayType));

		GpuVisibilityNonRenderableUniforms* unis =
			allocateAndBindUniforms<GpuVisibilityNonRenderableUniforms*>(sizeof(GpuVisibilityNonRenderableUniforms), cmdb, 0, 1);
		Array<Plane, 6> planes;
		extractClipPlanes(viewProjectionMat, planes);
		for(U32 i = 0; i < 6; ++i)
		{
			unis->m_clipPlanes[i] = Vec4(planes[i].getNormal().xyz(), planes[i].getOffset());
		}

		unis->m_feedbackCounterIdx = counterIdx;

		rgraph.bindStorageBuffer(0, 2, visibleIndicesBuffHandle);

		if(needsFeedback)
		{
			cmdb.bindStorageBuffer(0, 3, feedbackBuffer, feedbackBufferOffset, feedbackBufferRange);
			cmdb.bindStorageBuffer(0, 4, m_counterBuffers[counterBufferIdx].get(), 0, kMaxPtrSize);
		}

		dispatchPPCompute(cmdb, 64, 1, objCount, 1);
	});
}

} // end namespace anki
