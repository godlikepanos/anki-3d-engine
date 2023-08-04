// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Utils/GpuVisibility.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Scene/RenderStateBucket.h>
#include <AnKi/Scene/GpuSceneArray.h>
#include <AnKi/Core/GpuMemory/GpuVisibleTransientMemoryPool.h>
#include <AnKi/Core/GpuMemory/RebarTransientMemoryPool.h>
#include <AnKi/Core/GpuMemory/GpuSceneBuffer.h>
#include <AnKi/Collision/Functions.h>
#include <AnKi/Shaders/Include/GpuVisibilityTypes.h>
#include <AnKi/Core/StatsSet.h>

namespace anki {

static StatCounter g_visibleObjects(StatCategory::kRenderer, "Visible objects", StatFlag::kZeroEveryFrame);
static StatCounter g_testedObjects(StatCategory::kRenderer, "Visbility tested objects", StatFlag::kZeroEveryFrame);

Error GpuVisibility::init()
{
	for(MutatorValue hzb = 0; hzb < 2; ++hzb)
	{
		ANKI_CHECK(loadShaderProgram("ShaderBinaries/GpuVisibility.ankiprogbin",
									 Array<SubMutation, 3>{{{"HZB_TEST", hzb}, {"STATS", ANKI_STATS_ENABLED}, {"DISTANCE_TEST", 0}}}, m_prog,
									 m_frustumGrProgs[hzb]));
	}

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/GpuVisibility.ankiprogbin",
								 Array<SubMutation, 3>{{{"HZB_TEST", 0}, {"STATS", ANKI_STATS_ENABLED}, {"DISTANCE_TEST", 1}}}, m_prog,
								 m_distGrProg));

#if ANKI_STATS_ENABLED
	for(GpuReadbackMemoryAllocation& alloc : m_readbackMemory)
	{
		alloc = GpuReadbackMemoryPool::getSingleton().allocate(sizeof(U32));
	}
#endif

	return Error::kNone;
}

void GpuVisibility::populateRenderGraphInternal(Bool distanceBased, BaseGpuVisibilityInput& in, GpuVisibilityOutput& out)
{
	class DistanceTestData
	{
	public:
		Vec3 m_pointOfTest;
		F32 m_testRadius;
	};

	class FrustumTestData
	{
	public:
		RenderTargetHandle m_hzbRt;
		Mat4 m_viewProjMat;
	};

	FrustumTestData* frustumTestData = nullptr;
	DistanceTestData* distTestData = nullptr;

	if(distanceBased)
	{
		distTestData = newInstance<DistanceTestData>(getRenderer().getFrameMemoryPool());
		const DistanceGpuVisibilityInput& din = static_cast<DistanceGpuVisibilityInput&>(in);
		distTestData->m_pointOfTest = din.m_pointOfTest;
		distTestData->m_testRadius = din.m_testRadius;
	}
	else
	{
		frustumTestData = newInstance<FrustumTestData>(getRenderer().getFrameMemoryPool());
		const FrustumGpuVisibilityInput& fin = static_cast<FrustumGpuVisibilityInput&>(in);
		frustumTestData->m_viewProjMat = fin.m_viewProjectionMatrix;
	}

	U32 aabbCount = 0;
	switch(in.m_technique)
	{
	case RenderingTechnique::kGBuffer:
		aabbCount = GpuSceneArrays::RenderableAabbGBuffer::getSingleton().getElementCount();
	case RenderingTechnique::kDepth:
		aabbCount = GpuSceneArrays::RenderableAabbDepth::getSingleton().getElementCount();
		break;
	default:
		ANKI_ASSERT(0);
	}

	const U32 bucketCount = RenderStateBucketContainer::getSingleton().getBucketCount(in.m_technique);

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
	out.m_drawIndexedIndirectArgsBuffer = {indirectArgs.m_buffer, indirectArgs.m_offset, indirectArgs.m_size};

	const GpuVisibleTransientMemoryAllocation instanceRateRenderables =
		GpuVisibleTransientMemoryPool::getSingleton().allocate(aabbCount * sizeof(GpuSceneRenderable));
	out.m_instanceRateRenderablesBuffer = {instanceRateRenderables.m_buffer, instanceRateRenderables.m_offset, instanceRateRenderables.m_size};

	// Allocate and zero the MDI counts
	RebarAllocation mdiDrawCounts;
	U32* atomics = RebarTransientMemoryPool::getSingleton().allocateFrame<U32>(bucketCount, mdiDrawCounts);
	memset(atomics, 0, mdiDrawCounts.m_range);
	out.m_mdiDrawCountsBuffer = {&RebarTransientMemoryPool::getSingleton().getBuffer(), mdiDrawCounts.m_offset, mdiDrawCounts.m_range};

	// Import buffers
	out.m_mdiDrawCountsHandle = in.m_rgraph->importBuffer(&RebarTransientMemoryPool::getSingleton().getBuffer(), BufferUsageBit::kNone,
														  mdiDrawCounts.m_offset, mdiDrawCounts.m_range);

	// Create the renderpass
	ComputeRenderPassDescription& pass = in.m_rgraph->newComputeRenderPass(in.m_passesName);

	pass.newBufferDependency(getRenderer().getGpuSceneBufferHandle(), BufferUsageBit::kStorageComputeRead);
	pass.newBufferDependency(out.m_mdiDrawCountsHandle, BufferUsageBit::kStorageComputeWrite);

	if(!distanceBased && static_cast<FrustumGpuVisibilityInput&>(in).m_hzbRt)
	{
		frustumTestData->m_hzbRt = *static_cast<FrustumGpuVisibilityInput&>(in).m_hzbRt;
		pass.newTextureDependency(frustumTestData->m_hzbRt, TextureUsageBit::kSampledCompute);
	}

	pass.setWork([this, frustumTestData, distTestData, lodReferencePoint = in.m_lodReferencePoint, lodDistances = in.m_lodDistances,
				  technique = in.m_technique, mdiDrawCountsHandle = out.m_mdiDrawCountsHandle, instanceRateRenderables, indirectArgs, aabbCount
#if ANKI_STATS_ENABLED
				  ,
				  clearStatsBuffer, clearStatsBufferOffset, writeStatsBuffer, writeStatsBufferOffset
#endif
	](RenderPassWorkContext& rpass) {
		CommandBuffer& cmdb = *rpass.m_commandBuffer;

		if(frustumTestData)
		{
			cmdb.bindShaderProgram(m_frustumGrProgs[frustumTestData->m_hzbRt.isValid()].get());
		}
		else
		{
			cmdb.bindShaderProgram(m_distGrProg.get());
		}

		switch(technique)
		{
		case RenderingTechnique::kGBuffer:
			cmdb.bindStorageBuffer(0, 0, &GpuSceneBuffer::getSingleton().getBuffer(),
								   GpuSceneArrays::RenderableAabbGBuffer::getSingleton().getGpuSceneOffsetOfArrayBase(),
								   GpuSceneArrays::RenderableAabbGBuffer::getSingleton().getBufferRange());
			break;
		case RenderingTechnique::kDepth:
			cmdb.bindStorageBuffer(0, 0, &GpuSceneBuffer::getSingleton().getBuffer(),
								   GpuSceneArrays::RenderableAabbDepth::getSingleton().getGpuSceneOffsetOfArrayBase(),
								   GpuSceneArrays::RenderableAabbDepth::getSingleton().getBufferRange());
			break;
		default:
			ANKI_ASSERT(0);
		}

		cmdb.bindStorageBuffer(0, 1, &GpuSceneBuffer::getSingleton().getBuffer(),
							   GpuSceneArrays::Renderable::getSingleton().getGpuSceneOffsetOfArrayBase(),
							   GpuSceneArrays::Renderable::getSingleton().getBufferRange());

		cmdb.bindStorageBuffer(0, 2, &GpuSceneBuffer::getSingleton().getBuffer(), 0, kMaxPtrSize);

		cmdb.bindStorageBuffer(0, 3, instanceRateRenderables.m_buffer, instanceRateRenderables.m_offset, instanceRateRenderables.m_size);
		cmdb.bindStorageBuffer(0, 4, indirectArgs.m_buffer, indirectArgs.m_offset, indirectArgs.m_size);

		U32* offsets = allocateAndBindStorage<U32>(cmdb, 0, 5, RenderStateBucketContainer::getSingleton().getBucketCount(technique));
		U32 bucketCount = 0;
		U32 userCount = 0;
		RenderStateBucketContainer::getSingleton().iterateBuckets(technique, [&](const RenderStateInfo&, U32 userCount_) {
			offsets[bucketCount] = userCount;
			userCount += userCount_;
			++bucketCount;
		});
		ANKI_ASSERT(userCount == RenderStateBucketContainer::getSingleton().getBucketsItemCount(technique));

		rpass.bindStorageBuffer(0, 6, mdiDrawCountsHandle);

		if(frustumTestData)
		{
			FrustumGpuVisibilityUniforms* unis = allocateAndBindUniforms<FrustumGpuVisibilityUniforms>(cmdb, 0, 7);

			Array<Plane, 6> planes;
			extractClipPlanes(frustumTestData->m_viewProjMat, planes);
			for(U32 i = 0; i < 6; ++i)
			{
				unis->m_clipPlanes[i] = Vec4(planes[i].getNormal().xyz(), planes[i].getOffset());
			}

			ANKI_ASSERT(kMaxLodCount == 3);
			unis->m_maxLodDistances[0] = lodDistances[0];
			unis->m_maxLodDistances[1] = lodDistances[1];
			unis->m_maxLodDistances[2] = kMaxF32;
			unis->m_maxLodDistances[3] = kMaxF32;

			unis->m_lodReferencePoint = lodReferencePoint;
			unis->m_viewProjectionMat = frustumTestData->m_viewProjMat;

			if(frustumTestData->m_hzbRt.isValid())
			{
				rpass.bindColorTexture(0, 8, frustumTestData->m_hzbRt);
				cmdb.bindSampler(0, 9, getRenderer().getSamplers().m_nearestNearestClamp.get());
			}
		}
		else
		{
			DistanceGpuVisibilityUniforms unis;
			unis.m_pointOfTest = distTestData->m_pointOfTest;
			unis.m_testRadius = distTestData->m_testRadius;

			unis.m_maxLodDistances[0] = lodDistances[0];
			unis.m_maxLodDistances[1] = lodDistances[1];
			unis.m_maxLodDistances[2] = kMaxF32;
			unis.m_maxLodDistances[3] = kMaxF32;

			unis.m_lodReferencePoint = lodReferencePoint;

			cmdb.setPushConstants(&unis, sizeof(unis));
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

	return Error::kNone;
}

void GpuVisibilityNonRenderables::populateRenderGraph(GpuVisibilityNonRenderablesInput& in, GpuVisibilityNonRenderablesOutput& out)
{
	RenderGraphDescription& rgraph = *in.m_rgraph;

	U32 objCount = 0;
	switch(in.m_objectType)
	{
	case GpuSceneNonRenderableObjectType::kLight:
		objCount = GpuSceneArrays::Light::getSingleton().getElementCount();
		break;
	case GpuSceneNonRenderableObjectType::kDecal:
		objCount = GpuSceneArrays::Decal::getSingleton().getElementCount();
		break;
	case GpuSceneNonRenderableObjectType::kFogDensityVolume:
		objCount = GpuSceneArrays::FogDensityVolume::getSingleton().getElementCount();
		break;
	case GpuSceneNonRenderableObjectType::kGlobalIlluminationProbe:
		objCount = GpuSceneArrays::GlobalIlluminationProbe::getSingleton().getElementCount();
		break;
	case GpuSceneNonRenderableObjectType::kReflectionProbe:
		objCount = GpuSceneArrays::ReflectionProbe::getSingleton().getElementCount();
		break;
	default:
		ANKI_ASSERT(0);
	}

	if(objCount == 0)
	{
		return;
	}

	if(in.m_cpuFeedbackBuffer.m_buffer)
	{
		ANKI_ASSERT(in.m_cpuFeedbackBuffer.m_range == sizeof(U32) * (objCount * 2 + 1));
	}

	const Bool firstRunInFrame = m_lastFrameIdx != getRenderer().getFrameCount();
	if(firstRunInFrame)
	{
		// 1st run in this frame, do some bookkeeping
		m_lastFrameIdx = getRenderer().getFrameCount();
		m_counterBufferOffset = 0;
		m_counterBufferZeroingHandle = {};
	}

	constexpr U32 kCountersPerDispatch = 3; // 1 for the threadgroup, 1 for the visbile object count and 1 for objects with feedback
	const U32 counterBufferElementSize = getAlignedRoundUp(GrManager::getSingleton().getDeviceCapabilities().m_storageBufferBindOffsetAlignment,
														   U32(kCountersPerDispatch * sizeof(U32)));
	if(!m_counterBuffer.isCreated() || m_counterBufferOffset + counterBufferElementSize > m_counterBuffer->getSize()) [[unlikely]]
	{
		// Counter buffer not created or not big enough, create a new one

		BufferInitInfo buffInit("GpuVisibilityNonRenderablesCounters");
		buffInit.m_size = (m_counterBuffer.isCreated()) ? m_counterBuffer->getSize() * 2
														: kCountersPerDispatch * counterBufferElementSize * kInitialCounterArraySize;
		buffInit.m_usage = BufferUsageBit::kStorageComputeWrite | BufferUsageBit::kStorageComputeRead | BufferUsageBit::kTransferDestination;
		m_counterBuffer = GrManager::getSingleton().newBuffer(buffInit);

		m_counterBufferZeroingHandle = rgraph.importBuffer(m_counterBuffer.get(), buffInit.m_usage, 0, kMaxPtrSize);

		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("GpuVisibilityNonRenderablesClearCounterBuffer");

		pass.newBufferDependency(m_counterBufferZeroingHandle, BufferUsageBit::kTransferDestination);

		pass.setWork([counterBuffer = m_counterBuffer](RenderPassWorkContext& rgraph) {
			rgraph.m_commandBuffer->fillBuffer(counterBuffer.get(), 0, kMaxPtrSize, 0);
		});

		m_counterBufferOffset = 0;
	}
	else if(!firstRunInFrame)
	{
		m_counterBufferOffset += counterBufferElementSize;
	}

	// Allocate memory for the result
	GpuVisibleTransientMemoryAllocation visibleIndicesAlloc = GpuVisibleTransientMemoryPool::getSingleton().allocate((objCount + 1) * sizeof(U32));
	out.m_visiblesBuffer.m_buffer = visibleIndicesAlloc.m_buffer;
	out.m_visiblesBuffer.m_offset = visibleIndicesAlloc.m_offset;
	out.m_visiblesBuffer.m_range = visibleIndicesAlloc.m_size;
	out.m_visiblesBufferHandle =
		rgraph.importBuffer(out.m_visiblesBuffer.m_buffer, BufferUsageBit::kNone, out.m_visiblesBuffer.m_offset, out.m_visiblesBuffer.m_range);

	// Create the renderpass
	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass(in.m_passesName);

	pass.newBufferDependency(getRenderer().getGpuSceneBufferHandle(), BufferUsageBit::kStorageComputeRead);
	pass.newBufferDependency(out.m_visiblesBufferHandle, BufferUsageBit::kStorageComputeWrite);

	if(in.m_hzbRt)
	{
		pass.newTextureDependency(*in.m_hzbRt, TextureUsageBit::kSampledCompute);
	}

	if(m_counterBufferZeroingHandle.isValid()) [[unlikely]]
	{
		pass.newBufferDependency(m_counterBufferZeroingHandle, BufferUsageBit::kStorageComputeRead | BufferUsageBit::kStorageComputeWrite);
	}

	pass.setWork([this, objType = in.m_objectType, feedbackBuffer = in.m_cpuFeedbackBuffer, viewProjectionMat = in.m_viewProjectionMat,
				  visibleIndicesBuffHandle = out.m_visiblesBufferHandle, counterBuffer = m_counterBuffer, counterBufferOffset = m_counterBufferOffset,
				  objCount](RenderPassWorkContext& rgraph) {
		CommandBuffer& cmdb = *rgraph.m_commandBuffer;

		const Bool needsFeedback = feedbackBuffer.m_buffer != nullptr;

		cmdb.bindShaderProgram(m_grProgs[0][objType][needsFeedback].get());

		PtrSize objBufferOffset = 0;
		PtrSize objBufferRange = 0;
		switch(objType)
		{
		case GpuSceneNonRenderableObjectType::kLight:
			objBufferOffset = GpuSceneArrays::Light::getSingleton().getGpuSceneOffsetOfArrayBase();
			objBufferRange = GpuSceneArrays::Light::getSingleton().getElementCount() * GpuSceneArrays::Light::getSingleton().getElementSize();
			break;
		case GpuSceneNonRenderableObjectType::kDecal:
			objBufferOffset = GpuSceneArrays::Decal::getSingleton().getGpuSceneOffsetOfArrayBase();
			objBufferRange = GpuSceneArrays::Decal::getSingleton().getElementCount() * GpuSceneArrays::Decal::getSingleton().getElementSize();
			break;
		case GpuSceneNonRenderableObjectType::kFogDensityVolume:
			objBufferOffset = GpuSceneArrays::FogDensityVolume::getSingleton().getGpuSceneOffsetOfArrayBase();
			objBufferRange = GpuSceneArrays::FogDensityVolume::getSingleton().getElementCount()
							 * GpuSceneArrays::FogDensityVolume::getSingleton().getElementSize();
			break;
		case GpuSceneNonRenderableObjectType::kGlobalIlluminationProbe:
			objBufferOffset = GpuSceneArrays::GlobalIlluminationProbe::getSingleton().getGpuSceneOffsetOfArrayBase();
			objBufferRange = GpuSceneArrays::GlobalIlluminationProbe::getSingleton().getElementCount()
							 * GpuSceneArrays::GlobalIlluminationProbe::getSingleton().getElementSize();
			break;
		case GpuSceneNonRenderableObjectType::kReflectionProbe:
			objBufferOffset = GpuSceneArrays::ReflectionProbe::getSingleton().getGpuSceneOffsetOfArrayBase();
			objBufferRange =
				GpuSceneArrays::ReflectionProbe::getSingleton().getElementCount() * GpuSceneArrays::ReflectionProbe::getSingleton().getElementSize();
			break;
		default:
			ANKI_ASSERT(0);
		}
		cmdb.bindStorageBuffer(0, 0, &GpuSceneBuffer::getSingleton().getBuffer(), objBufferOffset, objBufferRange);

		GpuVisibilityNonRenderableUniforms unis;
		Array<Plane, 6> planes;
		extractClipPlanes(viewProjectionMat, planes);
		for(U32 i = 0; i < 6; ++i)
		{
			unis.m_clipPlanes[i] = Vec4(planes[i].getNormal().xyz(), planes[i].getOffset());
		}
		cmdb.setPushConstants(&unis, sizeof(unis));

		rgraph.bindStorageBuffer(0, 1, visibleIndicesBuffHandle);
		cmdb.bindStorageBuffer(0, 2, counterBuffer.get(), counterBufferOffset, sizeof(U32) * kCountersPerDispatch);

		if(needsFeedback)
		{
			cmdb.bindStorageBuffer(0, 3, feedbackBuffer.m_buffer, feedbackBuffer.m_offset, feedbackBuffer.m_range);
		}

		dispatchPPCompute(cmdb, 64, 1, objCount, 1);
	});
}

} // end namespace anki
