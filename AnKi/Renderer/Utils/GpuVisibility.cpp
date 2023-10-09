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

Error GpuVisibility::init()
{
	for(MutatorValue hzb = 0; hzb < 2; ++hzb)
	{
		for(MutatorValue gatherAabbs = 0; gatherAabbs < 2; ++gatherAabbs)
		{
			for(MutatorValue genHash = 0; genHash < 2; ++genHash)
			{
				ANKI_CHECK(loadShaderProgram(
					"ShaderBinaries/GpuVisibility.ankiprogbin",
					Array<SubMutation, 4>{{{"HZB_TEST", hzb}, {"DISTANCE_TEST", 0}, {"GATHER_AABBS", gatherAabbs}, {"HASH_VISIBLES", genHash}}},
					m_prog, m_frustumGrProgs[hzb][gatherAabbs][genHash]));
			}
		}
	}

	for(MutatorValue gatherAabbs = 0; gatherAabbs < 2; ++gatherAabbs)
	{
		for(MutatorValue genHash = 0; genHash < 2; ++genHash)
		{
			ANKI_CHECK(loadShaderProgram(
				"ShaderBinaries/GpuVisibility.ankiprogbin",
				Array<SubMutation, 4>{{{"HZB_TEST", 0}, {"DISTANCE_TEST", 1}, {"GATHER_AABBS", gatherAabbs}, {"HASH_VISIBLES", genHash}}}, m_prog,
				m_distGrProgs[gatherAabbs][genHash]));
		}
	}

	return Error::kNone;
}

void GpuVisibility::populateRenderGraphInternal(Bool distanceBased, BaseGpuVisibilityInput& in, GpuVisibilityOutput& out)
{
	ANKI_ASSERT(in.m_lodReferencePoint.x() != kMaxF32);

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
		aabbCount = GpuSceneArrays::RenderableBoundingVolumeGBuffer::getSingleton().getElementCount();
		break;
	case RenderingTechnique::kDepth:
		aabbCount = GpuSceneArrays::RenderableBoundingVolumeDepth::getSingleton().getElementCount();
		break;
	case RenderingTechnique::kForward:
		aabbCount = GpuSceneArrays::RenderableBoundingVolumeForward::getSingleton().getElementCount();
		break;
	default:
		ANKI_ASSERT(0);
	}

	if(aabbCount == 0) [[unlikely]]
	{
		// Early exit

		out.m_instanceRateRenderablesBuffer = GpuVisibleTransientMemoryPool::getSingleton().allocate(1 * sizeof(GpuSceneRenderable));
		out.m_drawIndexedIndirectArgsBuffer = GpuVisibleTransientMemoryPool::getSingleton().allocate(1 * sizeof(DrawIndexedIndirectArgs));

		U32* atomics;
		out.m_mdiDrawCountsBuffer = RebarTransientMemoryPool::getSingleton().allocateFrame<U32>(1, atomics);
		atomics[0] = 0;
		out.m_someBufferHandle = in.m_rgraph->importBuffer(BufferUsageBit::kNone, out.m_mdiDrawCountsBuffer);

		if(in.m_gatherAabbIndices)
		{
			U32* atomic;
			out.m_visibleAaabbIndicesBuffer = RebarTransientMemoryPool::getSingleton().allocateFrame<U32>(1, atomic);
			atomic[0] = 0;
		}

		return;
	}

	const U32 bucketCount = RenderStateBucketContainer::getSingleton().getBucketCount(in.m_technique);

	// Allocate memory for the indirect commands
	out.m_drawIndexedIndirectArgsBuffer = GpuVisibleTransientMemoryPool::getSingleton().allocate(aabbCount * sizeof(DrawIndexedIndirectArgs));
	out.m_instanceRateRenderablesBuffer = GpuVisibleTransientMemoryPool::getSingleton().allocate(aabbCount * sizeof(GpuSceneRenderableVertex));

	// Allocate memory for AABB indices
	if(in.m_gatherAabbIndices)
	{
		out.m_visibleAaabbIndicesBuffer = GpuVisibleTransientMemoryPool::getSingleton().allocate((aabbCount + 1) * sizeof(U32));
	}

	// Allocate memory for counters
	PtrSize counterMemory = 0;
	if(in.m_hashVisibles)
	{
		counterMemory += sizeof(GpuVisibilityHash);
		alignRoundUp(GrManager::getSingleton().getDeviceCapabilities().m_uavBufferBindOffsetAlignment, counterMemory);
	}

	const PtrSize mdiBufferOffset = counterMemory;
	const PtrSize mdiBufferSize = sizeof(U32) * bucketCount;
	counterMemory += mdiBufferSize;
	alignRoundUp(GrManager::getSingleton().getDeviceCapabilities().m_uavBufferBindOffsetAlignment, counterMemory);

	const BufferOffsetRange counterBuffer = GpuVisibleTransientMemoryPool::getSingleton().allocate(counterMemory);
	const BufferHandle counterBufferHandle = in.m_rgraph->importBuffer(BufferUsageBit::kNone, counterBuffer);
	out.m_someBufferHandle = counterBufferHandle;

	if(in.m_hashVisibles)
	{
		out.m_visiblesHashBuffer = {counterBuffer.m_buffer, counterBuffer.m_offset, sizeof(GpuVisibilityHash)};
	}

	// Zero some stuff
	{
		ComputeRenderPassDescription& pass = in.m_rgraph->newComputeRenderPass("GPU visibility: Zero stuff");
		pass.newBufferDependency(counterBufferHandle, BufferUsageBit::kTransferDestination);

		pass.setWork([counterBuffer, visibleAaabbIndicesBuffer = out.m_visibleAaabbIndicesBuffer](RenderPassWorkContext& rpass) {
			rpass.m_commandBuffer->fillBuffer(counterBuffer.m_buffer, counterBuffer.m_offset, counterBuffer.m_range, 0);

			if(visibleAaabbIndicesBuffer.m_buffer)
			{
				rpass.m_commandBuffer->fillBuffer(visibleAaabbIndicesBuffer.m_buffer, visibleAaabbIndicesBuffer.m_offset, sizeof(U32), 0);
			}
		});
	}

	// Set the MDI count buffer
	out.m_mdiDrawCountsBuffer = {counterBuffer.m_buffer, counterBuffer.m_offset + mdiBufferOffset, mdiBufferSize};

	// Create the renderpass
	ComputeRenderPassDescription& pass = in.m_rgraph->newComputeRenderPass(in.m_passesName);

	pass.newBufferDependency(getRenderer().getGpuSceneBufferHandle(), BufferUsageBit::kUavComputeRead);
	pass.newBufferDependency(counterBufferHandle, BufferUsageBit::kUavComputeWrite);

	if(!distanceBased && static_cast<FrustumGpuVisibilityInput&>(in).m_hzbRt)
	{
		frustumTestData->m_hzbRt = *static_cast<FrustumGpuVisibilityInput&>(in).m_hzbRt;
		pass.newTextureDependency(frustumTestData->m_hzbRt, TextureUsageBit::kSampledCompute);
	}

	pass.setWork([this, frustumTestData, distTestData, lodReferencePoint = in.m_lodReferencePoint, lodDistances = in.m_lodDistances,
				  technique = in.m_technique, mdiDrawCountsBuffer = out.m_mdiDrawCountsBuffer,
				  instanceRateRenderables = out.m_instanceRateRenderablesBuffer, indirectArgs = out.m_drawIndexedIndirectArgsBuffer, aabbCount,
				  visibleAabbsBuffer = out.m_visibleAaabbIndicesBuffer, hashBuffer = out.m_visiblesHashBuffer](RenderPassWorkContext& rpass) {
		CommandBuffer& cmdb = *rpass.m_commandBuffer;

		const Bool gatherAabbIndices = visibleAabbsBuffer.m_buffer != nullptr;
		const Bool genHash = hashBuffer.m_buffer != nullptr;

		if(frustumTestData)
		{
			cmdb.bindShaderProgram(m_frustumGrProgs[frustumTestData->m_hzbRt.isValid()][gatherAabbIndices][genHash].get());
		}
		else
		{
			cmdb.bindShaderProgram(m_distGrProgs[gatherAabbIndices][genHash].get());
		}

		BufferOffsetRange aabbsBuffer;
		switch(technique)
		{
		case RenderingTechnique::kGBuffer:
			aabbsBuffer = GpuSceneArrays::RenderableBoundingVolumeGBuffer::getSingleton().getBufferOffsetRange();
			break;
		case RenderingTechnique::kDepth:
			aabbsBuffer = GpuSceneArrays::RenderableBoundingVolumeDepth::getSingleton().getBufferOffsetRange();
			break;
		case RenderingTechnique::kForward:
			aabbsBuffer = GpuSceneArrays::RenderableBoundingVolumeForward::getSingleton().getBufferOffsetRange();
			break;
		default:
			ANKI_ASSERT(0);
		}

		cmdb.bindUavBuffer(0, 0, aabbsBuffer);
		cmdb.bindUavBuffer(0, 1, GpuSceneArrays::Renderable::getSingleton().getBufferOffsetRange());
		cmdb.bindUavBuffer(0, 2, &GpuSceneBuffer::getSingleton().getBuffer(), 0, kMaxPtrSize);
		cmdb.bindUavBuffer(0, 3, instanceRateRenderables);
		cmdb.bindUavBuffer(0, 4, indirectArgs);

		U32* offsets = allocateAndBindUav<U32>(cmdb, 0, 5, RenderStateBucketContainer::getSingleton().getBucketCount(technique));
		U32 bucketCount = 0;
		U32 userCount = 0;
		RenderStateBucketContainer::getSingleton().iterateBuckets(technique, [&](const RenderStateInfo&, U32 userCount_) {
			offsets[bucketCount] = userCount;
			userCount += userCount_;
			++bucketCount;
		});
		ANKI_ASSERT(userCount == RenderStateBucketContainer::getSingleton().getBucketsItemCount(technique));

		cmdb.bindUavBuffer(0, 6, mdiDrawCountsBuffer);

		if(frustumTestData)
		{
			FrustumGpuVisibilityConstants* unis = allocateAndBindConstants<FrustumGpuVisibilityConstants>(cmdb, 0, 7);

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
			DistanceGpuVisibilityConstants unis;
			unis.m_pointOfTest = distTestData->m_pointOfTest;
			unis.m_testRadius = distTestData->m_testRadius;

			unis.m_maxLodDistances[0] = lodDistances[0];
			unis.m_maxLodDistances[1] = lodDistances[1];
			unis.m_maxLodDistances[2] = kMaxF32;
			unis.m_maxLodDistances[3] = kMaxF32;

			unis.m_lodReferencePoint = lodReferencePoint;

			cmdb.setPushConstants(&unis, sizeof(unis));
		}

		if(gatherAabbIndices)
		{
			cmdb.bindUavBuffer(0, 12, visibleAabbsBuffer);
		}

		if(genHash)
		{
			cmdb.bindUavBuffer(0, 13, hashBuffer);
		}

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
	ANKI_ASSERT(in.m_viewProjectionMat != Mat4::getZero());
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
		U32* count;
		out.m_visiblesBuffer = RebarTransientMemoryPool::getSingleton().allocateFrame(sizeof(U32), count);
		*count = 0;
		out.m_visiblesBufferHandle = rgraph.importBuffer(BufferUsageBit::kNone, out.m_visiblesBuffer);

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
	const U32 counterBufferElementSize =
		getAlignedRoundUp(GrManager::getSingleton().getDeviceCapabilities().m_uavBufferBindOffsetAlignment, U32(kCountersPerDispatch * sizeof(U32)));
	if(!m_counterBuffer.isCreated() || m_counterBufferOffset + counterBufferElementSize > m_counterBuffer->getSize()) [[unlikely]]
	{
		// Counter buffer not created or not big enough, create a new one

		BufferInitInfo buffInit("GpuVisibilityNonRenderablesCounters");
		buffInit.m_size = (m_counterBuffer.isCreated()) ? m_counterBuffer->getSize() * 2
														: kCountersPerDispatch * counterBufferElementSize * kInitialCounterArraySize;
		buffInit.m_usage = BufferUsageBit::kUavComputeWrite | BufferUsageBit::kUavComputeRead | BufferUsageBit::kTransferDestination;
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
	out.m_visiblesBuffer = GpuVisibleTransientMemoryPool::getSingleton().allocate((objCount + 1) * sizeof(U32));
	out.m_visiblesBufferHandle = rgraph.importBuffer(BufferUsageBit::kNone, out.m_visiblesBuffer);

	// Create the renderpass
	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass(in.m_passesName);

	pass.newBufferDependency(getRenderer().getGpuSceneBufferHandle(), BufferUsageBit::kUavComputeRead);
	pass.newBufferDependency(out.m_visiblesBufferHandle, BufferUsageBit::kUavComputeWrite);

	if(in.m_hzbRt)
	{
		pass.newTextureDependency(*in.m_hzbRt, TextureUsageBit::kSampledCompute);
	}

	if(m_counterBufferZeroingHandle.isValid()) [[unlikely]]
	{
		pass.newBufferDependency(m_counterBufferZeroingHandle, BufferUsageBit::kUavComputeRead | BufferUsageBit::kUavComputeWrite);
	}

	pass.setWork([this, objType = in.m_objectType, feedbackBuffer = in.m_cpuFeedbackBuffer, viewProjectionMat = in.m_viewProjectionMat,
				  visibleIndicesBuffHandle = out.m_visiblesBufferHandle, counterBuffer = m_counterBuffer, counterBufferOffset = m_counterBufferOffset,
				  objCount](RenderPassWorkContext& rgraph) {
		CommandBuffer& cmdb = *rgraph.m_commandBuffer;

		const Bool needsFeedback = feedbackBuffer.m_buffer != nullptr;

		cmdb.bindShaderProgram(m_grProgs[0][objType][needsFeedback].get());

		BufferOffsetRange objBuffer;
		switch(objType)
		{
		case GpuSceneNonRenderableObjectType::kLight:
			objBuffer = GpuSceneArrays::Light::getSingleton().getBufferOffsetRange();
			break;
		case GpuSceneNonRenderableObjectType::kDecal:
			objBuffer = GpuSceneArrays::Decal::getSingleton().getBufferOffsetRange();
			break;
		case GpuSceneNonRenderableObjectType::kFogDensityVolume:
			objBuffer = GpuSceneArrays::FogDensityVolume::getSingleton().getBufferOffsetRange();
			break;
		case GpuSceneNonRenderableObjectType::kGlobalIlluminationProbe:
			objBuffer = GpuSceneArrays::GlobalIlluminationProbe::getSingleton().getBufferOffsetRange();
			break;
		case GpuSceneNonRenderableObjectType::kReflectionProbe:
			objBuffer = GpuSceneArrays::ReflectionProbe::getSingleton().getBufferOffsetRange();
			break;
		default:
			ANKI_ASSERT(0);
		}
		cmdb.bindUavBuffer(0, 0, objBuffer);

		GpuVisibilityNonRenderableConstants unis;
		Array<Plane, 6> planes;
		extractClipPlanes(viewProjectionMat, planes);
		for(U32 i = 0; i < 6; ++i)
		{
			unis.m_clipPlanes[i] = Vec4(planes[i].getNormal().xyz(), planes[i].getOffset());
		}
		cmdb.setPushConstants(&unis, sizeof(unis));

		rgraph.bindUavBuffer(0, 1, visibleIndicesBuffHandle);
		cmdb.bindUavBuffer(0, 2, counterBuffer.get(), counterBufferOffset, sizeof(U32) * kCountersPerDispatch);

		if(needsFeedback)
		{
			cmdb.bindUavBuffer(0, 3, feedbackBuffer.m_buffer, feedbackBuffer.m_offset, feedbackBuffer.m_range);
		}

		dispatchPPCompute(cmdb, 64, 1, objCount, 1);
	});
}

Error GpuVisibilityAccelerationStructures::init()
{
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/GpuVisibilityAccelerationStructures.ankiprogbin", m_visibilityProg, m_visibilityGrProg));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/GpuVisibilityAccelerationStructuresZeroRemainingInstances.ankiprogbin", m_zeroRemainingInstancesProg,
								 m_zeroRemainingInstancesGrProg));

	BufferInitInfo inf("GpuVisibilityAccelerationStructuresCounters");
	inf.m_size = sizeof(U32) * 2;
	inf.m_usage = BufferUsageBit::kUavComputeWrite | BufferUsageBit::kUavComputeRead | BufferUsageBit::kTransferDestination;
	m_counterBuffer = GrManager::getSingleton().newBuffer(inf);

	zeroBuffer(m_counterBuffer.get());

	return Error::kNone;
}

void GpuVisibilityAccelerationStructures::pupulateRenderGraph(GpuVisibilityAccelerationStructuresInput& in,
															  GpuVisibilityAccelerationStructuresOutput& out)
{
	in.validate();
	RenderGraphDescription& rgraph = *in.m_rgraph;

#if ANKI_ASSERTIONS_ENABLED
	ANKI_ASSERT(m_lastFrameIdx != getRenderer().getFrameCount());
	m_lastFrameIdx = getRenderer().getFrameCount();
#endif

	// Allocate the transient buffers
	const U32 aabbCount = GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount();

	out.m_instancesBuffer = GpuVisibleTransientMemoryPool::getSingleton().allocate(aabbCount * sizeof(AccelerationStructureInstance));
	out.m_someBufferHandle = rgraph.importBuffer(BufferUsageBit::kUavComputeWrite, out.m_instancesBuffer);

	out.m_renderableIndicesBuffer = GpuVisibleTransientMemoryPool::getSingleton().allocate((aabbCount + 1) * sizeof(U32));

	const BufferOffsetRange zeroInstancesDispatchArgsBuff = GpuVisibleTransientMemoryPool::getSingleton().allocate(sizeof(DispatchIndirectArgs));

	// Create vis pass
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass(in.m_passesName);

		pass.newBufferDependency(getRenderer().getGpuSceneBufferHandle(), BufferUsageBit::kUavComputeRead);
		pass.newBufferDependency(out.m_someBufferHandle, BufferUsageBit::kUavComputeWrite);

		pass.setWork([this, viewProjMat = in.m_viewProjectionMatrix, lodDistances = in.m_lodDistances, pointOfTest = in.m_pointOfTest,
					  testRadius = in.m_testRadius, instancesBuff = out.m_instancesBuffer, indicesBuff = out.m_renderableIndicesBuffer,
					  zeroInstancesDispatchArgsBuff](RenderPassWorkContext& rgraph) {
			CommandBuffer& cmdb = *rgraph.m_commandBuffer;

			cmdb.bindShaderProgram(m_visibilityGrProg.get());

			GpuVisibilityAccelerationStructuresConstants unis;
			Array<Plane, 6> planes;
			extractClipPlanes(viewProjMat, planes);
			for(U32 i = 0; i < 6; ++i)
			{
				unis.m_clipPlanes[i] = Vec4(planes[i].getNormal().xyz(), planes[i].getOffset());
			}

			unis.m_pointOfTest = pointOfTest;
			unis.m_testRadius = testRadius;

			ANKI_ASSERT(kMaxLodCount == 3);
			unis.m_maxLodDistances[0] = lodDistances[0];
			unis.m_maxLodDistances[1] = lodDistances[1];
			unis.m_maxLodDistances[2] = kMaxF32;
			unis.m_maxLodDistances[3] = kMaxF32;

			cmdb.setPushConstants(&unis, sizeof(unis));

			cmdb.bindUavBuffer(0, 0, GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getBufferOffsetRange());
			cmdb.bindUavBuffer(0, 1, GpuSceneArrays::Renderable::getSingleton().getBufferOffsetRange());
			cmdb.bindUavBuffer(0, 2, &GpuSceneBuffer::getSingleton().getBuffer(), 0, kMaxPtrSize);
			cmdb.bindUavBuffer(0, 3, instancesBuff);
			cmdb.bindUavBuffer(0, 4, indicesBuff);
			cmdb.bindUavBuffer(0, 5, m_counterBuffer.get(), 0, sizeof(U32) * 2);
			cmdb.bindUavBuffer(0, 6, zeroInstancesDispatchArgsBuff);

			const U32 aabbCount = GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount();
			dispatchPPCompute(cmdb, 64, 1, aabbCount, 1);
		});
	}

	// Zero remaining instances
	{
		Array<Char, 64> passName;
		snprintf(passName.getBegin(), sizeof(passName), "%s: Zero remaining instances", in.m_passesName.cstr());

		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass(passName.getBegin());

		pass.newBufferDependency(out.m_someBufferHandle, BufferUsageBit::kUavComputeWrite);

		pass.setWork([this, zeroInstancesDispatchArgsBuff, instancesBuff = out.m_instancesBuffer,
					  indicesBuff = out.m_renderableIndicesBuffer](RenderPassWorkContext& rgraph) {
			CommandBuffer& cmdb = *rgraph.m_commandBuffer;

			cmdb.bindShaderProgram(m_zeroRemainingInstancesGrProg.get());

			cmdb.bindUavBuffer(0, 0, indicesBuff);
			cmdb.bindUavBuffer(0, 1, instancesBuff);

			cmdb.dispatchComputeIndirect(zeroInstancesDispatchArgsBuff.m_buffer, zeroInstancesDispatchArgsBuff.m_offset);
		});
	}
}

} // end namespace anki
