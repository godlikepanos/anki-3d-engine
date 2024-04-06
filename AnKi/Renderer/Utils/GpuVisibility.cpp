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
#include <AnKi/Core/GpuMemory/UnifiedGeometryBuffer.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Core/CVarSet.h>

namespace anki {

constexpr U32 kMaxVisibleObjects = 30 * 1024;

constexpr U32 kMaxVisiblePrimitives = 40'000'000;
constexpr U32 kMaxVisibleMeshlets = kMaxVisiblePrimitives / kMaxPrimitivesPerMeshlet;
constexpr PtrSize kMaxMeshletMemory = kMaxVisibleMeshlets * sizeof(GpuSceneMeshletInstance);

constexpr U32 kVisibleMaxMeshletGroups = max(kMaxVisibleObjects, (kMaxVisibleMeshlets + kMeshletGroupSize - 1) / kMeshletGroupSize);
constexpr PtrSize kMaxMeshletGroupMemory = kVisibleMaxMeshletGroups * sizeof(GpuSceneMeshletGroupInstance);

static NumericCVar<PtrSize> g_maxMeshletMemoryPerTest(CVarSubsystem::kRenderer, "MaxMeshletMemoryPerTest", kMaxMeshletMemory, 1_KB, 100_MB,
													  "Max memory that will be allocated per GPU occlusion test for storing meshlets");
static NumericCVar<PtrSize> g_maxMeshletGroupMemoryPerTest(CVarSubsystem::kRenderer, "MaxMeshletGroupMemoryPerTest", kMaxMeshletGroupMemory, 1_KB,
														   100_MB,
														   "Max memory that will be allocated per GPU occlusion test for storing meshlet groups");

static StatCounter g_gpuVisMemoryAllocatedStatVar(StatCategory::kRenderer, "GPU visibility mem",
												  StatFlag::kBytes | StatFlag::kMainThreadUpdates | StatFlag::kZeroEveryFrame);

static BufferOffsetRange allocateTransientGpuMem(PtrSize size)
{
	BufferOffsetRange out = {};

	if(size)
	{
		g_gpuVisMemoryAllocatedStatVar.increment(size);
		out = GpuVisibleTransientMemoryPool::getSingleton().allocate(size);
	}

	return out;
}

Error GpuVisibility::init()
{
	for(MutatorValue hzb = 0; hzb < 2; ++hzb)
	{
		for(MutatorValue gatherAabbs = 0; gatherAabbs < 2; ++gatherAabbs)
		{
			for(MutatorValue genHash = 0; genHash < 2; ++genHash)
			{
				for(MutatorValue gatherType = 0; gatherType < 3; ++gatherType)
				{
					ANKI_CHECK(loadShaderProgram("ShaderBinaries/GpuVisibility.ankiprogbin",
												 {{"HZB_TEST", hzb},
												  {"DISTANCE_TEST", 0},
												  {"GATHER_AABBS", gatherAabbs},
												  {"HASH_VISIBLES", genHash},
												  {"GATHER_TYPE", gatherType + 1}},
												 m_prog, m_frustumGrProgs[hzb][gatherAabbs][genHash][gatherType]));
				}
			}
		}
	}

	for(MutatorValue gatherAabbs = 0; gatherAabbs < 2; ++gatherAabbs)
	{
		for(MutatorValue genHash = 0; genHash < 2; ++genHash)
		{
			for(MutatorValue gatherType = 0; gatherType < 3; ++gatherType)
			{
				ANKI_CHECK(loadShaderProgram("ShaderBinaries/GpuVisibility.ankiprogbin",
											 {{"HZB_TEST", 0},
											  {"DISTANCE_TEST", 1},
											  {"GATHER_AABBS", gatherAabbs},
											  {"HASH_VISIBLES", genHash},
											  {"GATHER_TYPE", gatherType + 1}},
											 m_prog, m_distGrProgs[gatherAabbs][genHash][gatherType]));
			}
		}
	}

	for(MutatorValue hzb = 0; hzb < 2; ++hzb)
	{
		for(MutatorValue passthrough = 0; passthrough < 2; ++passthrough)
		{
			ANKI_CHECK(loadShaderProgram("ShaderBinaries/GpuVisibilityMeshlet.ankiprogbin", {{"HZB_TEST", hzb}, {"PASSTHROUGH", passthrough}},
										 m_meshletCullingProg, m_meshletCullingGrProgs[hzb][passthrough]));
		}
	}

	return Error::kNone;
}

void GpuVisibility::computeGpuVisibilityMemoryRequirements(RenderingTechnique t, MemoryRequirements& total, WeakArray<MemoryRequirements> perBucket)
{
	ANKI_ASSERT(perBucket.getSize() == RenderStateBucketContainer::getSingleton().getBucketCount(t));

	U32 totalMeshletCount = 0;
	U32 totalMeshletGroupCount = 0;
	U32 totalRenderableCount = 0;

	RenderStateBucketContainer::getSingleton().iterateBuckets(t, [&](const RenderStateInfo&, U32 userCount, U32 meshletGroupCount, U32 meshletCount) {
		if(meshletCount)
		{
			totalMeshletCount += meshletCount;
			totalMeshletGroupCount += meshletGroupCount;
		}
		else
		{
			totalRenderableCount += userCount;
		}
	});

	const U32 maxVisibleMeshlets = min(U32(g_maxMeshletMemoryPerTest.get() / sizeof(GpuSceneMeshletInstance)), totalMeshletCount);
	const U32 maxVisibleMeshletGroups = min(U32(g_maxMeshletGroupMemoryPerTest.get() / sizeof(GpuSceneMeshletGroupInstance)), totalMeshletGroupCount);
	const U32 maxVisibleRenderables = min(kMaxVisibleObjects, totalRenderableCount);

	total = {};

	U32 bucketCount = 0;
	RenderStateBucketContainer::getSingleton().iterateBuckets(t, [&](const RenderStateInfo&, U32 userCount, U32 meshletGroupCount, U32 meshletCount) {
		MemoryRequirements& bucket = perBucket[bucketCount++];

		// Use U64 cause some expressions are overflowing

		if(meshletCount)
		{
			ANKI_ASSERT(meshletGroupCount > 0);

			ANKI_ASSERT(totalMeshletCount > 0);
			bucket.m_meshletInstanceCount = max(1u, U32(U64(meshletCount) * maxVisibleMeshlets / totalMeshletCount));

			ANKI_ASSERT(totalMeshletGroupCount > 0);
			bucket.m_meshletGroupInstanceCount = max(1u, U32(U64(meshletGroupCount) * maxVisibleMeshletGroups / totalMeshletGroupCount));
		}
		else if(userCount > 0)
		{
			ANKI_ASSERT(totalRenderableCount > 0);
			bucket.m_renderableInstanceCount = max(1u, U32(U64(userCount) * maxVisibleRenderables / totalRenderableCount));
		}

		total.m_meshletInstanceCount += bucket.m_meshletInstanceCount;
		total.m_meshletGroupInstanceCount += bucket.m_meshletGroupInstanceCount;
		total.m_renderableInstanceCount += bucket.m_renderableInstanceCount;
	});
}

void GpuVisibility::populateRenderGraphInternal(Bool distanceBased, BaseGpuVisibilityInput& in, GpuVisibilityOutput& out)
{
	ANKI_ASSERT(in.m_lodReferencePoint.x() != kMaxF32);

	if(RenderStateBucketContainer::getSingleton().getBucketsActiveUserCount(in.m_technique) == 0) [[unlikely]]
	{
		// Early exit
		in = {};
		return;
	}

	RenderGraphDescription& rgraph = *in.m_rgraph;

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
		UVec2 m_finalRenderTargetSize;
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
		frustumTestData->m_finalRenderTargetSize = fin.m_viewportSize;
	}

	// Allocate memory
	const Bool firstCallInFrame = m_runCtx.m_frameIdx != getRenderer().getFrameCount();
	if(firstCallInFrame)
	{
		// First call in frame. Init stuff

		m_runCtx.m_frameIdx = getRenderer().getFrameCount();
		m_runCtx.m_populateRenderGraphCallCount = 0;
		m_runCtx.m_populateRenderGraphMeshletRenderingCallCount = 0;

		// Calc memory requirements
		MemoryRequirements maxTotalMemReq;
		WeakArray<MemoryRequirements> bucketsMemReqs;
		for(RenderingTechnique t : EnumBitsIterable<RenderingTechnique, RenderingTechniqueBit>(RenderingTechniqueBit::kAllRaster))
		{
			const U32 tBucketCount = RenderStateBucketContainer::getSingleton().getBucketCount(t);
			if(tBucketCount == 0)
			{
				continue;
			}

			newArray<MemoryRequirements>(getRenderer().getFrameMemoryPool(), tBucketCount, bucketsMemReqs);

			computeGpuVisibilityMemoryRequirements(t, m_runCtx.m_totalMemRequirements[t], bucketsMemReqs);

			maxTotalMemReq = maxTotalMemReq.max(m_runCtx.m_totalMemRequirements[t]);

			newArray<InstanceRange>(getRenderer().getFrameMemoryPool(), tBucketCount, m_runCtx.m_renderableInstanceRanges[t]);
			newArray<InstanceRange>(getRenderer().getFrameMemoryPool(), tBucketCount, m_runCtx.m_meshletGroupInstanceRanges[t]);
			newArray<InstanceRange>(getRenderer().getFrameMemoryPool(), tBucketCount, m_runCtx.m_meshletInstanceRanges[t]);

			U32 renderablesFirstInstance = 0, groupsFirstInstance = 0, meshletsFirstInstance = 0;
			for(U32 i = 0; i < tBucketCount; ++i)
			{
				m_runCtx.m_renderableInstanceRanges[t][i].m_firstInstance = renderablesFirstInstance;
				m_runCtx.m_renderableInstanceRanges[t][i].m_instanceCount = bucketsMemReqs[i].m_renderableInstanceCount;

				m_runCtx.m_meshletGroupInstanceRanges[t][i].m_firstInstance = groupsFirstInstance;
				m_runCtx.m_meshletGroupInstanceRanges[t][i].m_instanceCount = bucketsMemReqs[i].m_meshletGroupInstanceCount;

				m_runCtx.m_meshletInstanceRanges[t][i].m_firstInstance = meshletsFirstInstance;
				m_runCtx.m_meshletInstanceRanges[t][i].m_instanceCount = bucketsMemReqs[i].m_meshletInstanceCount;

				renderablesFirstInstance += bucketsMemReqs[i].m_renderableInstanceCount;
				groupsFirstInstance += bucketsMemReqs[i].m_meshletGroupInstanceCount;
				meshletsFirstInstance += bucketsMemReqs[i].m_meshletInstanceCount;
			}
		}

		// Allocate persistent memory
		for(PersistentMemory& mem : m_runCtx.m_persistentMem)
		{
			mem = {};

			mem.m_drawIndexedIndirectArgsBuffer = allocateTransientGpuMem(maxTotalMemReq.m_renderableInstanceCount * sizeof(DrawIndexedIndirectArgs));
			mem.m_renderableInstancesBuffer = allocateTransientGpuMem(maxTotalMemReq.m_renderableInstanceCount * sizeof(GpuSceneRenderableInstance));

			mem.m_meshletGroupsInstancesBuffer =
				allocateTransientGpuMem(maxTotalMemReq.m_meshletGroupInstanceCount * sizeof(GpuSceneMeshletGroupInstance));

			mem.m_bufferDepedency =
				rgraph.importBuffer(BufferUsageBit::kNone, (mem.m_drawIndexedIndirectArgsBuffer.m_buffer) ? mem.m_drawIndexedIndirectArgsBuffer
																										  : mem.m_meshletGroupsInstancesBuffer);
		}

		if(getRenderer().runSoftwareMeshletRendering())
		{
			// Because someone will need it later

			for(PersistentMemoryMeshletRendering& mem : m_runCtx.m_persistentMeshletRenderingMem)
			{
				mem = {};

				mem.m_meshletInstancesBuffer = allocateTransientGpuMem(maxTotalMemReq.m_meshletInstanceCount * sizeof(GpuSceneMeshletInstance));

				mem.m_bufferDepedency = rgraph.importBuffer(BufferUsageBit::kNone, mem.m_meshletInstancesBuffer);
			}
		}
	}

	const U32 bucketCount = RenderStateBucketContainer::getSingleton().getBucketCount(in.m_technique);
	const MemoryRequirements& req = m_runCtx.m_totalMemRequirements[in.m_technique];
	const PersistentMemory& mem = m_runCtx.m_persistentMem[m_runCtx.m_populateRenderGraphCallCount++ % m_runCtx.m_persistentMem.getSize()];

	out.m_legacy.m_drawIndexedIndirectArgsBuffer = mem.m_drawIndexedIndirectArgsBuffer;
	out.m_legacy.m_drawIndexedIndirectArgsBuffer.m_range = req.m_renderableInstanceCount * sizeof(DrawIndexedIndirectArgs);

	out.m_legacy.m_renderableInstancesBuffer = mem.m_renderableInstancesBuffer;
	out.m_legacy.m_renderableInstancesBuffer.m_range = req.m_renderableInstanceCount * sizeof(GpuSceneRenderableInstance);

	out.m_legacy.m_mdiDrawCountsBuffer = allocateTransientGpuMem(sizeof(U32) * bucketCount);

	out.m_mesh.m_meshletGroupInstancesBuffer = mem.m_meshletGroupsInstancesBuffer;
	out.m_mesh.m_meshletGroupInstancesBuffer.m_range = req.m_meshletGroupInstanceCount * sizeof(GpuSceneMeshletGroupInstance);

	out.m_mesh.m_taskShaderIndirectArgsBuffer = allocateTransientGpuMem(bucketCount * sizeof(DispatchIndirectArgs));

	if(in.m_hashVisibles)
	{
		out.m_visiblesHashBuffer = allocateTransientGpuMem(sizeof(GpuVisibilityHash));
	}

	if(in.m_gatherAabbIndices)
	{
		out.m_visibleAaabbIndicesBuffer =
			allocateTransientGpuMem((RenderStateBucketContainer::getSingleton().getBucketsActiveUserCount(in.m_technique) + 1) * sizeof(U32));
	}

	// Set instance sub-ranges
	out.m_legacy.m_bucketRenderableInstanceRanges = m_runCtx.m_renderableInstanceRanges[in.m_technique];
	out.m_mesh.m_bucketMeshletGroupInstanceRanges = m_runCtx.m_meshletGroupInstanceRanges[in.m_technique];

	// Zero some stuff
	const BufferHandle zeroStuffDependency = rgraph.importBuffer(BufferUsageBit::kNone, out.m_legacy.m_mdiDrawCountsBuffer);
	{
		Array<Char, 128> passName;
		snprintf(passName.getBegin(), passName.getSizeInBytes(), "GPU vis zero: %s", in.m_passesName.cstr());

		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass(passName.getBegin());
		pass.newBufferDependency(zeroStuffDependency, BufferUsageBit::kTransferDestination);

		pass.setWork([out](RenderPassWorkContext& rpass) {
			CommandBuffer& cmdb = *rpass.m_commandBuffer;

			cmdb.pushDebugMarker("MDI counts", Vec3(1.0f, 1.0f, 1.0f));
			cmdb.fillBuffer(out.m_legacy.m_mdiDrawCountsBuffer, 0);
			cmdb.popDebugMarker();

			if(out.m_mesh.m_taskShaderIndirectArgsBuffer.m_buffer)
			{
				cmdb.pushDebugMarker("Task shader indirect args", Vec3(1.0f, 1.0f, 1.0f));
				cmdb.fillBuffer(out.m_mesh.m_taskShaderIndirectArgsBuffer, 0);
				cmdb.popDebugMarker();
			}

			if(out.m_visiblesHashBuffer.m_buffer)
			{
				cmdb.pushDebugMarker("Visibles hash", Vec3(1.0f, 1.0f, 1.0f));
				cmdb.fillBuffer(out.m_visiblesHashBuffer, 0);
				cmdb.popDebugMarker();
			}

			if(out.m_visibleAaabbIndicesBuffer.m_buffer)
			{
				cmdb.pushDebugMarker("Visible AABB indices", Vec3(1.0f, 1.0f, 1.0f));
				cmdb.fillBuffer(out.m_visibleAaabbIndicesBuffer.m_buffer, out.m_visibleAaabbIndicesBuffer.m_offset, sizeof(U32), 0);
				cmdb.popDebugMarker();
			}
		});
	}

	// Set the out dependency. Use one of the big buffers.
	out.m_dependency = mem.m_bufferDepedency;

	// Create the renderpass
	Array<Char, 128> passName;
	snprintf(passName.getBegin(), passName.getSizeInBytes(), "GPU vis: %s", in.m_passesName.cstr());

	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass(passName.getBegin());

	pass.newBufferDependency(getRenderer().getGpuSceneBufferHandle(), BufferUsageBit::kUavComputeRead);
	pass.newBufferDependency(zeroStuffDependency, BufferUsageBit::kUavComputeWrite);
	pass.newBufferDependency(out.m_dependency, BufferUsageBit::kUavComputeWrite);

	if(!distanceBased && static_cast<FrustumGpuVisibilityInput&>(in).m_hzbRt)
	{
		frustumTestData->m_hzbRt = *static_cast<FrustumGpuVisibilityInput&>(in).m_hzbRt;
		pass.newTextureDependency(frustumTestData->m_hzbRt, TextureUsageBit::kSampledCompute);
	}

	pass.setWork([this, frustumTestData, distTestData, lodReferencePoint = in.m_lodReferencePoint, lodDistances = in.m_lodDistances,
				  technique = in.m_technique, out](RenderPassWorkContext& rpass) {
		CommandBuffer& cmdb = *rpass.m_commandBuffer;

		const Bool gatherAabbIndices = out.m_visibleAaabbIndicesBuffer.m_buffer != nullptr;
		const Bool genHash = out.m_visiblesHashBuffer.m_buffer != nullptr;

		U32 gatherType = 0;
		if(out.m_mesh.m_meshletGroupInstancesBuffer.m_range > 0)
		{
			gatherType |= 2u;
		}

		if(out.m_legacy.m_renderableInstancesBuffer.m_range > 0)
		{
			gatherType |= 1u;
		}
		ANKI_ASSERT(gatherType != 0);

		if(frustumTestData)
		{
			cmdb.bindShaderProgram(m_frustumGrProgs[frustumTestData->m_hzbRt.isValid()][gatherAabbIndices][genHash][gatherType - 1u].get());
		}
		else
		{
			cmdb.bindShaderProgram(m_distGrProgs[gatherAabbIndices][genHash][gatherType - 1u].get());
		}

		BufferOffsetRange aabbsBuffer;
		U32 aabbCount = 0;
		switch(technique)
		{
		case RenderingTechnique::kGBuffer:
			aabbsBuffer = GpuSceneArrays::RenderableBoundingVolumeGBuffer::getSingleton().getBufferOffsetRange();
			aabbCount = GpuSceneArrays::RenderableBoundingVolumeGBuffer::getSingleton().getElementCount();
			break;
		case RenderingTechnique::kDepth:
			aabbsBuffer = GpuSceneArrays::RenderableBoundingVolumeDepth::getSingleton().getBufferOffsetRange();
			aabbCount = GpuSceneArrays::RenderableBoundingVolumeDepth::getSingleton().getElementCount();
			break;
		case RenderingTechnique::kForward:
			aabbsBuffer = GpuSceneArrays::RenderableBoundingVolumeForward::getSingleton().getBufferOffsetRange();
			aabbCount = GpuSceneArrays::RenderableBoundingVolumeForward::getSingleton().getElementCount();
			break;
		default:
			ANKI_ASSERT(0);
		}

		cmdb.bindUavBuffer(0, 0, aabbsBuffer);
		cmdb.bindUavBuffer(0, 1, GpuSceneArrays::Renderable::getSingleton().getBufferOffsetRange());
		cmdb.bindUavBuffer(0, 2, GpuSceneArrays::MeshLod::getSingleton().getBufferOffsetRange());
		cmdb.bindUavBuffer(0, 3, GpuSceneArrays::Transform::getSingleton().getBufferOffsetRange());
		cmdb.bindUavBuffer(0, 4, GpuSceneBuffer::getSingleton().getBufferOffsetRange());
		if(gatherType & 1u)
		{
			cmdb.bindUavBuffer(0, 5, out.m_legacy.m_renderableInstancesBuffer);
			cmdb.bindUavBuffer(0, 6, out.m_legacy.m_drawIndexedIndirectArgsBuffer);
			cmdb.bindUavBuffer(0, 7, out.m_legacy.m_mdiDrawCountsBuffer);
		}
		if(gatherType & 2u)
		{
			cmdb.bindUavBuffer(0, 8, out.m_mesh.m_taskShaderIndirectArgsBuffer);
			cmdb.bindUavBuffer(0, 9, out.m_mesh.m_meshletGroupInstancesBuffer);
		}

		const U32 bucketCount = RenderStateBucketContainer::getSingleton().getBucketCount(technique);
		UVec2* instanceRanges = allocateAndBindUav<UVec2>(cmdb, 0, 10, bucketCount);
		for(U32 i = 0; i < bucketCount; ++i)
		{
			const Bool legacyBucket = m_runCtx.m_renderableInstanceRanges[technique][i].m_instanceCount > 0;

			if(legacyBucket)
			{
				instanceRanges[i].x() = m_runCtx.m_renderableInstanceRanges[technique][i].m_firstInstance;
				instanceRanges[i].y() = m_runCtx.m_renderableInstanceRanges[technique][i].m_instanceCount;
			}
			else
			{
				instanceRanges[i].x() = m_runCtx.m_meshletGroupInstanceRanges[technique][i].m_firstInstance;
				instanceRanges[i].y() = m_runCtx.m_meshletGroupInstanceRanges[technique][i].m_instanceCount;
			}
		}

		if(frustumTestData)
		{
			FrustumGpuVisibilityConstants* unis = allocateAndBindConstants<FrustumGpuVisibilityConstants>(cmdb, 0, 11);

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
			unis->m_finalRenderTargetSize = Vec2(frustumTestData->m_finalRenderTargetSize);

			if(frustumTestData->m_hzbRt.isValid())
			{
				rpass.bindColorTexture(0, 12, frustumTestData->m_hzbRt);
				cmdb.bindSampler(0, 13, getRenderer().getSamplers().m_nearestNearestClamp.get());
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
			cmdb.bindUavBuffer(0, 14, out.m_visibleAaabbIndicesBuffer);
		}

		if(genHash)
		{
			cmdb.bindUavBuffer(0, 15, out.m_visiblesHashBuffer);
		}

		dispatchPPCompute(cmdb, 64, 1, aabbCount, 1);
	});
}

void GpuVisibility::populateRenderGraphMeshletInternal(Bool passthrough, BaseGpuMeshletVisibilityInput& in, GpuMeshletVisibilityOutput& out)
{
	RenderGraphDescription& rgraph = *in.m_rgraph;

	if(in.m_taskShaderIndirectArgsBuffer.m_buffer == nullptr) [[unlikely]]
	{
		// Early exit
		return;
	}

	class NonPassthrough
	{
	public:
		Mat4 m_viewProjectionMatrix;
		Mat3x4 m_cameraTransform;

		UVec2 m_viewportSize;

		RenderTargetHandle m_hzbRt;
	}* nonPassthroughData = nullptr;

	if(!passthrough)
	{
		GpuMeshletVisibilityInput& nonPassthroughIn = static_cast<GpuMeshletVisibilityInput&>(in);

		nonPassthroughData = newInstance<NonPassthrough>(getRenderer().getFrameMemoryPool());
		nonPassthroughData->m_viewProjectionMatrix = nonPassthroughIn.m_viewProjectionMatrix;
		nonPassthroughData->m_cameraTransform = nonPassthroughIn.m_cameraTransform;
		nonPassthroughData->m_viewportSize = nonPassthroughIn.m_viewportSize;
		nonPassthroughData->m_hzbRt = nonPassthroughIn.m_hzbRt;
	}

	// Allocate memory
	const U32 bucketCount = m_runCtx.m_renderableInstanceRanges[in.m_technique].getSize();
	ANKI_ASSERT(RenderStateBucketContainer::getSingleton().getBucketCount(in.m_technique) == bucketCount);

	const PersistentMemoryMeshletRendering& mem = m_runCtx.m_persistentMeshletRenderingMem[m_runCtx.m_populateRenderGraphMeshletRenderingCallCount++
																						   % m_runCtx.m_persistentMeshletRenderingMem.getSize()];

	out.m_drawIndirectArgsBuffer = allocateTransientGpuMem(sizeof(DrawIndirectArgs) * bucketCount);

	out.m_meshletInstancesBuffer = mem.m_meshletInstancesBuffer;
	out.m_meshletInstancesBuffer.m_range = m_runCtx.m_totalMemRequirements[in.m_technique].m_meshletInstanceCount * sizeof(GpuSceneMeshletInstance);

	out.m_bucketMeshletInstanceRanges = m_runCtx.m_meshletInstanceRanges[in.m_technique];

	// Zero some stuff
	const BufferHandle indirectArgsDep = rgraph.importBuffer(BufferUsageBit::kNone, out.m_drawIndirectArgsBuffer);
	{
		Array<Char, 128> passName;
		snprintf(passName.getBegin(), passName.getSizeInBytes(), "GPU meshlet vis zero: %s", in.m_passesName.cstr());

		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass(passName.getBegin());
		pass.newBufferDependency(indirectArgsDep, BufferUsageBit::kTransferDestination);

		pass.setWork([drawIndirectArgsBuffer = out.m_drawIndirectArgsBuffer](RenderPassWorkContext& rpass) {
			CommandBuffer& cmdb = *rpass.m_commandBuffer;

			cmdb.pushDebugMarker("Draw indirect args", Vec3(1.0f, 1.0f, 1.0f));
			cmdb.fillBuffer(drawIndirectArgsBuffer, 0);
			cmdb.popDebugMarker();
		});
	}

	out.m_dependency = mem.m_bufferDepedency;

	// Create the renderpass
	Array<Char, 128> passName;
	snprintf(passName.getBegin(), passName.getSizeInBytes(), "GPU meshlet vis: %s", in.m_passesName.cstr());

	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass(passName.getBegin());

	pass.newBufferDependency(indirectArgsDep, BufferUsageBit::kUavComputeWrite);
	pass.newBufferDependency(mem.m_bufferDepedency, BufferUsageBit::kUavComputeWrite);
	pass.newBufferDependency(in.m_dependency, BufferUsageBit::kIndirectCompute);

	pass.setWork([this, nonPassthroughData, computeIndirectArgs = in.m_taskShaderIndirectArgsBuffer, out,
				  meshletGroupInstancesBuffer = in.m_meshletGroupInstancesBuffer,
				  bucketMeshletGroupInstanceRanges = in.m_bucketMeshletGroupInstanceRanges](RenderPassWorkContext& rpass) {
		CommandBuffer& cmdb = *rpass.m_commandBuffer;

		const U32 bucketCount = out.m_bucketMeshletInstanceRanges.getSize();

		for(U32 i = 0; i < bucketCount; ++i)
		{
			if(out.m_bucketMeshletInstanceRanges[i].m_instanceCount == 0)
			{
				continue;
			}

			const Bool hasHzb = (nonPassthroughData) ? nonPassthroughData->m_hzbRt.isValid() : false;
			const Bool isPassthrough = (nonPassthroughData == nullptr);

			cmdb.bindShaderProgram(m_meshletCullingGrProgs[hasHzb][isPassthrough].get());

			cmdb.bindUavBuffer(0, 0, meshletGroupInstancesBuffer);
			cmdb.bindUavBuffer(0, 1, GpuSceneArrays::Renderable::getSingleton().getBufferOffsetRange());
			cmdb.bindUavBuffer(0, 2, GpuSceneArrays::MeshLod::getSingleton().getBufferOffsetRange());
			cmdb.bindUavBuffer(0, 3, GpuSceneArrays::Transform::getSingleton().getBufferOffsetRange());
			cmdb.bindUavBuffer(0, 4, UnifiedGeometryBuffer::getSingleton().getBufferOffsetRange());
			cmdb.bindUavBuffer(0, 5, out.m_drawIndirectArgsBuffer);
			cmdb.bindUavBuffer(0, 6, out.m_meshletInstancesBuffer);
			if(hasHzb)
			{
				rpass.bindColorTexture(0, 7, nonPassthroughData->m_hzbRt);
				cmdb.bindSampler(0, 8, getRenderer().getSamplers().m_nearestNearestClamp.get());
			}

			class Consts
			{
			public:
				Mat4 m_viewProjectionMatrix;

				Vec3 m_cameraPos;
				U32 m_firstDrawArg;

				Vec2 m_viewportSizef;
				U32 m_firstMeshletGroup;
				U32 m_firstMeshlet;

				U32 m_meshletCount;
				U32 m_padding1;
				U32 m_padding2;
				U32 m_padding3;
			} consts;
			consts.m_viewProjectionMatrix = (!isPassthrough) ? nonPassthroughData->m_viewProjectionMatrix : Mat4::getIdentity();
			consts.m_cameraPos = (!isPassthrough) ? nonPassthroughData->m_cameraTransform.getTranslationPart().xyz() : Vec3(0.0f);
			consts.m_firstDrawArg = i;
			consts.m_viewportSizef = (!isPassthrough) ? Vec2(nonPassthroughData->m_viewportSize) : Vec2(0.0f);
			consts.m_firstMeshletGroup = bucketMeshletGroupInstanceRanges[i].getFirstInstance();
			consts.m_firstMeshlet = out.m_bucketMeshletInstanceRanges[i].getFirstInstance();
			consts.m_meshletCount = out.m_bucketMeshletInstanceRanges[i].getInstanceCount();
			cmdb.setPushConstants(&consts, sizeof(consts));

			cmdb.dispatchComputeIndirect(computeIndirectArgs.m_buffer, computeIndirectArgs.m_offset + i * sizeof(DispatchIndirectArgs));
		};
	});
}

Error GpuVisibilityNonRenderables::init()
{
	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/GpuVisibilityNonRenderables.ankiprogbin", m_prog));

	for(MutatorValue hzb = 0; hzb < 2; ++hzb)
	{
		for(GpuSceneNonRenderableObjectType type : EnumIterable<GpuSceneNonRenderableObjectType>())
		{
			for(MutatorValue cpuFeedback = 0; cpuFeedback < 2; ++cpuFeedback)
			{
				ANKI_CHECK(loadShaderProgram("ShaderBinaries/GpuVisibilityNonRenderables.ankiprogbin",
											 {{"HZB_TEST", hzb}, {"OBJECT_TYPE", MutatorValue(type)}, {"CPU_FEEDBACK", cpuFeedback}}, m_prog,
											 m_grProgs[hzb][type][cpuFeedback]));
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
	out.m_visiblesBuffer = allocateTransientGpuMem((objCount + 1) * sizeof(U32));
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

	out.m_instancesBuffer = allocateTransientGpuMem(aabbCount * sizeof(AccelerationStructureInstance));
	out.m_someBufferHandle = rgraph.importBuffer(BufferUsageBit::kUavComputeWrite, out.m_instancesBuffer);

	out.m_renderableIndicesBuffer = allocateTransientGpuMem((aabbCount + 1) * sizeof(U32));

	const BufferOffsetRange zeroInstancesDispatchArgsBuff = allocateTransientGpuMem(sizeof(DispatchIndirectArgs));

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
			cmdb.bindUavBuffer(0, 2, GpuSceneArrays::MeshLod::getSingleton().getBufferOffsetRange());
			cmdb.bindUavBuffer(0, 3, GpuSceneArrays::Transform::getSingleton().getBufferOffsetRange());
			cmdb.bindUavBuffer(0, 4, instancesBuff);
			cmdb.bindUavBuffer(0, 5, indicesBuff);
			cmdb.bindUavBuffer(0, 6, m_counterBuffer.get(), 0, sizeof(U32) * 2);
			cmdb.bindUavBuffer(0, 7, zeroInstancesDispatchArgsBuff);

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
