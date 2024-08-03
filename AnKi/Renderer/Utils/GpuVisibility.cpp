// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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

static StatCounter g_gpuVisMemoryAllocatedStatVar(StatCategory::kRenderer, "GPU vis mem",
												  StatFlag::kBytes | StatFlag::kMainThreadUpdates | StatFlag::kZeroEveryFrame);

static StatCounter g_maxGpuVisMemoryAllocatedStatVar(StatCategory::kRenderer, "GPU vis mem: max ever used/frame",
													 StatFlag::kBytes | StatFlag::kMainThreadUpdates);

class GpuVisLimits
{
public:
	U32 m_maxVisibleLegacyRenderables = 0;
	U32 m_totalLegacyRenderables = 0;

	U32 m_maxVisibleMeshlets = 0;
};

static GpuVisLimits computeLimits(RenderingTechnique t)
{
	GpuVisLimits out;

	const RenderStateBucketContainer& buckets = RenderStateBucketContainer::getSingleton();

	const U32 meshletUserCount = buckets.getBucketsActiveUserCountWithMeshletSupport(t);
	ANKI_ASSERT(meshletUserCount == 0 || (g_meshletRenderingCVar.get() || GrManager::getSingleton().getDeviceCapabilities().m_meshShaders));
	out.m_totalLegacyRenderables = buckets.getBucketsActiveUserCountWithNoMeshletSupport(t);
	out.m_maxVisibleLegacyRenderables = min(out.m_totalLegacyRenderables, kMaxVisibleObjects);

	out.m_maxVisibleMeshlets = (meshletUserCount) ? min(kMaxVisibleMeshlets, buckets.getBucketsLod0MeshletCount(t)) : 0;

	return out;
}

class GpuVisMemoryStats : public RendererObject, public MakeSingletonSimple<GpuVisMemoryStats>
{
public:
	void informAboutAllocation(PtrSize size)
	{
		if(m_frameIdx != getRenderer().getFrameCount())
		{
			// First call in the frame, update the stat var

			m_frameIdx = getRenderer().getFrameCount();

			m_maxMemUsedInFrame = max(m_maxMemUsedInFrame, m_memUsedThisFrame);
			m_memUsedThisFrame = 0;
			g_maxGpuVisMemoryAllocatedStatVar.set(m_maxMemUsedInFrame);
		}

		m_memUsedThisFrame += size;
	}

private:
	PtrSize m_memUsedThisFrame = 0;
	PtrSize m_maxMemUsedInFrame = 0;
	U64 m_frameIdx = kMaxU64;
};

BufferView allocateTransientGpuMem(PtrSize size)
{
	BufferView out = {};

	if(size)
	{
		g_gpuVisMemoryAllocatedStatVar.increment(size);
		out = GpuVisibleTransientMemoryPool::getSingleton().allocate(size);

		GpuVisMemoryStats::getSingleton().informAboutAllocation(size);
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
				for(MutatorValue gatherMeshlets = 0; gatherMeshlets < 2; ++gatherMeshlets)
				{
					ANKI_CHECK(loadShaderProgram("ShaderBinaries/GpuVisibilityStage1.ankiprogbin",
												 {{"HZB_TEST", hzb},
												  {"DISTANCE_TEST", 0},
												  {"GATHER_AABBS", gatherAabbs},
												  {"HASH_VISIBLES", genHash},
												  {"GATHER_MESHLETS", gatherMeshlets}},
												 m_1stStageProg, m_frustumGrProgs[hzb][gatherAabbs][genHash][gatherMeshlets]));
				}
			}
		}
	}

	for(MutatorValue gatherAabbs = 0; gatherAabbs < 2; ++gatherAabbs)
	{
		for(MutatorValue genHash = 0; genHash < 2; ++genHash)
		{
			for(MutatorValue gatherMeshlets = 0; gatherMeshlets < 2; ++gatherMeshlets)
			{
				ANKI_CHECK(loadShaderProgram("ShaderBinaries/GpuVisibilityStage1.ankiprogbin",
											 {{"HZB_TEST", 0},
											  {"DISTANCE_TEST", 1},
											  {"GATHER_AABBS", gatherAabbs},
											  {"HASH_VISIBLES", genHash},
											  {"GATHER_MESHLETS", gatherMeshlets}},
											 m_1stStageProg, m_distGrProgs[gatherAabbs][genHash][gatherMeshlets]));
			}
		}
	}

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/GpuVisibilityStage2.ankiprogbin", {{"HZB_TEST", 0}, {"PASSTHROUGH", 0}, {"MESH_SHADERS", 0}},
								 m_2ndStageProg, m_gatherGrProg, "Legacy"));

	for(MutatorValue hzb = 0; hzb < 2; ++hzb)
	{
		for(MutatorValue passthrough = 0; passthrough < 2; ++passthrough)
		{
			for(MutatorValue meshShaders = 0; meshShaders < 2; ++meshShaders)
			{
				ANKI_CHECK(loadShaderProgram("ShaderBinaries/GpuVisibilityStage2.ankiprogbin",
											 {{"HZB_TEST", hzb}, {"PASSTHROUGH", passthrough}, {"MESH_SHADERS", meshShaders}}, m_2ndStageProg,
											 m_meshletGrProgs[hzb][passthrough][meshShaders], "Meshlets"));
			}
		}
	}

	return Error::kNone;
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

	RenderGraphBuilder& rgraph = *in.m_rgraph;

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

		if(fin.m_hzbRt)
		{
			frustumTestData->m_hzbRt = *fin.m_hzbRt;
		}
	}

	const Bool firstCallInFrame = m_persistentMemory.m_frameIdx != getRenderer().getFrameCount();
	if(firstCallInFrame)
	{
		m_persistentMemory.m_frameIdx = getRenderer().getFrameCount();
	}

	// OoM
	if(firstCallInFrame)
	{
		U32 data;
		PtrSize dataReadSize;
		getRenderer().getReadbackManager().readMostRecentData(m_outOfMemoryReadback, &data, sizeof(data), dataReadSize);

		if(dataReadSize == sizeof(U32) && data != 0)
		{
			CString who;
			switch(data)
			{
			case 0b1:
				who = "Stage 1";
				break;
			case 0b10:
				who = "Stage 2";
				break;
			case 0b11:
				who = "Both stages";
				break;
			default:
				ANKI_ASSERT(0);
			}

			ANKI_RESOURCE_LOGE("GPU visibility went out of memory: %s", who.cstr());
		}

		getRenderer().getReadbackManager().allocateData(m_outOfMemoryReadback, sizeof(U32), m_outOfMemoryReadbackBuffer);
	}

	// Get some limits
	const RenderStateBucketContainer& buckets = RenderStateBucketContainer::getSingleton();
	const U32 bucketCount = buckets.getBucketCount(in.m_technique);
	const GpuVisLimits limits = computeLimits(in.m_technique);

	const Bool bHwMeshletRendering = GrManager::getSingleton().getDeviceCapabilities().m_meshShaders && limits.m_maxVisibleMeshlets > 0;
	const Bool bSwMeshletRendering = g_meshletRenderingCVar.get() && !bHwMeshletRendering && limits.m_maxVisibleMeshlets > 0;
	const Bool bMeshletRendering = bHwMeshletRendering || bSwMeshletRendering;
	const Bool bLegacyRendering = limits.m_maxVisibleLegacyRenderables > 0;

	// Allocate persistent memory for the frame
	if(firstCallInFrame)
	{
		GpuVisLimits maxLimits;
		for(RenderingTechnique t : EnumBitsIterable<RenderingTechnique, RenderingTechniqueBit>(RenderingTechniqueBit::kAllRaster))
		{
			const GpuVisLimits limits = computeLimits(t);
			maxLimits.m_maxVisibleLegacyRenderables = max(maxLimits.m_maxVisibleLegacyRenderables, limits.m_maxVisibleLegacyRenderables);
			maxLimits.m_maxVisibleMeshlets = max(maxLimits.m_maxVisibleMeshlets, limits.m_maxVisibleMeshlets);
		}

		m_persistentMemory.m_stage1.m_visibleRenderables =
			allocateTransientGpuMem(sizeof(GpuVisibilityVisibleRenderableDesc) * maxLimits.m_maxVisibleLegacyRenderables);
		m_persistentMemory.m_stage1.m_visibleMeshlets =
			allocateTransientGpuMem(sizeof(GpuVisibilityVisibleMeshletDesc) * maxLimits.m_maxVisibleMeshlets);

		m_persistentMemory.m_stage2Legacy.m_instanceRateRenderables =
			allocateTransientGpuMem(sizeof(UVec4) * maxLimits.m_maxVisibleLegacyRenderables);
		m_persistentMemory.m_stage2Legacy.m_drawIndexedIndirectArgs =
			allocateTransientGpuMem(sizeof(DrawIndexedIndirectArgs) * maxLimits.m_maxVisibleLegacyRenderables);

		m_persistentMemory.m_stage2Meshlet.m_meshletInstances =
			allocateTransientGpuMem(sizeof(GpuSceneMeshletInstance) * maxLimits.m_maxVisibleMeshlets);

		m_persistentMemory.m_dep = rgraph.importBuffer((bMeshletRendering) ? m_persistentMemory.m_stage1.m_visibleMeshlets
																		   : m_persistentMemory.m_stage1.m_visibleRenderables,
													   BufferUsageBit::kNone);
	}

	// Compute the MDI sub-ranges
	if(limits.m_maxVisibleLegacyRenderables)
	{
		newArray<InstanceRange>(getRenderer().getFrameMemoryPool(), bucketCount, out.m_legacy.m_bucketIndirectArgsRanges);

		U32 ibucket = 0;
		U32 offset = 0;
		buckets.iterateBuckets(in.m_technique, [&](const RenderStateInfo&, U32 userCount, U32 meshletCount) {
			out.m_legacy.m_bucketIndirectArgsRanges[ibucket].m_firstInstance = offset;

			if(meshletCount == 0 && userCount > 0)
			{
				out.m_legacy.m_bucketIndirectArgsRanges[ibucket].m_instanceCount =
					max(1u, U32(U64(userCount) * limits.m_maxVisibleLegacyRenderables / limits.m_totalLegacyRenderables));

				offset += out.m_legacy.m_bucketIndirectArgsRanges[ibucket].m_instanceCount;
			}

			++ibucket;
		});

		// The last element should point to the limit of the buffer
		InstanceRange& last = out.m_legacy.m_bucketIndirectArgsRanges.getBack();
		ANKI_ASSERT(limits.m_maxVisibleLegacyRenderables >= last.m_firstInstance);
		last.m_instanceCount = limits.m_maxVisibleLegacyRenderables - last.m_firstInstance;
	}

	// Allocate memory for stage 1
	class Stage1Mem
	{
	public:
		BufferView m_counters;
		BufferView m_visibleRenderables;
		BufferView m_visibleMeshlets;

		BufferView m_renderablePrefixSums;
		BufferView m_meshletPrefixSums;
		BufferView m_stage2IndirectArgs;

		BufferView m_visibleAabbIndices;
		BufferView m_hash;
	} stage1Mem;

	stage1Mem.m_counters = allocateTransientGpuMem(sizeof(U32) * 3);
	if(in.m_limitMemory)
	{
		PtrSize newRange = sizeof(GpuVisibilityVisibleRenderableDesc) * limits.m_maxVisibleLegacyRenderables;
		if(newRange)
		{
			ANKI_ASSERT(newRange <= m_persistentMemory.m_stage1.m_visibleRenderables.getRange());
			stage1Mem.m_visibleRenderables = BufferView(m_persistentMemory.m_stage1.m_visibleRenderables).setRange(newRange);
		}

		newRange = sizeof(GpuVisibilityVisibleMeshletDesc) * limits.m_maxVisibleMeshlets;
		if(newRange)
		{
			ANKI_ASSERT(newRange <= m_persistentMemory.m_stage1.m_visibleMeshlets.getRange());
			stage1Mem.m_visibleMeshlets = BufferView(m_persistentMemory.m_stage1.m_visibleMeshlets).setRange(newRange);
		}
	}
	else
	{
		stage1Mem.m_visibleRenderables = allocateTransientGpuMem(sizeof(GpuVisibilityVisibleRenderableDesc) * limits.m_maxVisibleLegacyRenderables);
		stage1Mem.m_visibleMeshlets = allocateTransientGpuMem(sizeof(GpuVisibilityVisibleMeshletDesc) * limits.m_maxVisibleMeshlets);
	}
	stage1Mem.m_renderablePrefixSums = allocateTransientGpuMem(sizeof(U32) * bucketCount);
	stage1Mem.m_meshletPrefixSums = allocateTransientGpuMem(sizeof(U32) * bucketCount);
	stage1Mem.m_stage2IndirectArgs = allocateTransientGpuMem(sizeof(DispatchIndirectArgs) * 2);

	if(in.m_gatherAabbIndices)
	{
		stage1Mem.m_visibleAabbIndices = allocateTransientGpuMem(sizeof(U32) * buckets.getBucketsActiveUserCount(in.m_technique));
	}

	if(in.m_hashVisibles)
	{
		stage1Mem.m_hash = allocateTransientGpuMem(sizeof(GpuVisibilityHash));
	}

	// Allocate memory for stage 2
	class Stage2Mem
	{
	public:
		class
		{
		public:
			BufferView m_instanceRateRenderables;
			BufferView m_drawIndexedIndirectArgs;

			BufferView m_mdiDrawCounts;
		} m_legacy;

		class
		{
		public:
			BufferView m_indirectDrawArgs;
			BufferView m_dispatchMeshIndirectArgs;

			BufferView m_meshletInstances;
		} m_meshlet;
	} stage2Mem;

	if(bLegacyRendering)
	{
		if(in.m_limitMemory)
		{
			PtrSize newRange = sizeof(UVec4) * limits.m_maxVisibleLegacyRenderables;
			ANKI_ASSERT(newRange <= m_persistentMemory.m_stage2Legacy.m_instanceRateRenderables.getRange());
			stage2Mem.m_legacy.m_instanceRateRenderables = BufferView(m_persistentMemory.m_stage2Legacy.m_instanceRateRenderables).setRange(newRange);

			newRange = sizeof(DrawIndexedIndirectArgs) * limits.m_maxVisibleLegacyRenderables;
			ANKI_ASSERT(newRange <= m_persistentMemory.m_stage2Legacy.m_drawIndexedIndirectArgs.getRange());
			stage2Mem.m_legacy.m_drawIndexedIndirectArgs = BufferView(m_persistentMemory.m_stage2Legacy.m_drawIndexedIndirectArgs).setRange(newRange);
		}
		else
		{
			stage2Mem.m_legacy.m_instanceRateRenderables = allocateTransientGpuMem(sizeof(UVec4) * limits.m_maxVisibleLegacyRenderables);
			stage2Mem.m_legacy.m_drawIndexedIndirectArgs =
				allocateTransientGpuMem(sizeof(DrawIndexedIndirectArgs) * limits.m_maxVisibleLegacyRenderables);
		}

		stage2Mem.m_legacy.m_mdiDrawCounts = allocateTransientGpuMem(sizeof(U32) * bucketCount);
	}

	if(bMeshletRendering)
	{
		if(bHwMeshletRendering)
		{
			stage2Mem.m_meshlet.m_dispatchMeshIndirectArgs = allocateTransientGpuMem(sizeof(DispatchIndirectArgs) * bucketCount);
		}
		else
		{
			stage2Mem.m_meshlet.m_indirectDrawArgs = allocateTransientGpuMem(sizeof(DrawIndirectArgs) * bucketCount);
		}

		const PtrSize newRange = sizeof(GpuSceneMeshletInstance) * limits.m_maxVisibleMeshlets;
		if(in.m_limitMemory)
		{
			ANKI_ASSERT(newRange <= m_persistentMemory.m_stage2Meshlet.m_meshletInstances.getRange());
			stage2Mem.m_meshlet.m_meshletInstances = BufferView(m_persistentMemory.m_stage2Meshlet.m_meshletInstances).setRange(newRange);
		}
		else
		{
			stage2Mem.m_meshlet.m_meshletInstances = allocateTransientGpuMem(newRange);
		}
	}

	// Setup output
	out.m_legacy.m_renderableInstancesBuffer = stage2Mem.m_legacy.m_instanceRateRenderables;
	out.m_legacy.m_mdiDrawCountsBuffer = stage2Mem.m_legacy.m_mdiDrawCounts;
	out.m_legacy.m_drawIndexedIndirectArgsBuffer = stage2Mem.m_legacy.m_drawIndexedIndirectArgs;
	out.m_mesh.m_dispatchMeshIndirectArgsBuffer = stage2Mem.m_meshlet.m_dispatchMeshIndirectArgs;
	out.m_mesh.m_drawIndirectArgs = stage2Mem.m_meshlet.m_indirectDrawArgs;
	out.m_mesh.m_meshletInstancesBuffer = stage2Mem.m_meshlet.m_meshletInstances;
	out.m_visibleAaabbIndicesBuffer = stage1Mem.m_visibleAabbIndices;
	out.m_visiblesHashBuffer = stage1Mem.m_hash;
	if(bHwMeshletRendering)
	{
		out.m_mesh.m_firstMeshletBuffer = stage1Mem.m_meshletPrefixSums;
	}

	// Use one buffer as a depedency. Doesn't matter which
	out.m_dependency = (in.m_limitMemory) ? m_persistentMemory.m_dep : rgraph.importBuffer(stage1Mem.m_stage2IndirectArgs, BufferUsageBit::kNone);

	// Zero some stuff
	const BufferHandle zeroMemDep = rgraph.importBuffer(stage1Mem.m_counters, BufferUsageBit::kNone);
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass(generateTempPassName("GPU vis zero: %s", in.m_passesName.cstr()));
		pass.newBufferDependency(zeroMemDep, BufferUsageBit::kTransferDestination);

		pass.setWork([stage1Mem, stage2Mem, this](RenderPassWorkContext& rpass) {
			CommandBuffer& cmdb = *rpass.m_commandBuffer;

			cmdb.pushDebugMarker("Temp counters", Vec3(1.0f, 1.0f, 1.0f));
			cmdb.fillBuffer(stage1Mem.m_counters, 0);
			cmdb.popDebugMarker();

			if(stage1Mem.m_renderablePrefixSums.isValid())
			{
				cmdb.pushDebugMarker("Renderable prefix sums", Vec3(1.0f, 1.0f, 1.0f));
				cmdb.fillBuffer(stage1Mem.m_renderablePrefixSums, 0);
				cmdb.popDebugMarker();
			}

			if(stage1Mem.m_meshletPrefixSums.isValid())
			{
				cmdb.pushDebugMarker("Meshlet prefix sums", Vec3(1.0f, 1.0f, 1.0f));
				cmdb.fillBuffer(stage1Mem.m_meshletPrefixSums, 0);
				cmdb.popDebugMarker();
			}

			if(stage2Mem.m_legacy.m_drawIndexedIndirectArgs.isValid())
			{
				cmdb.pushDebugMarker("Draw indexed indirect args", Vec3(1.0f, 1.0f, 1.0f));
				cmdb.fillBuffer(stage2Mem.m_legacy.m_drawIndexedIndirectArgs, 0);
				cmdb.popDebugMarker();
			}

			if(stage2Mem.m_meshlet.m_dispatchMeshIndirectArgs.isValid())
			{
				cmdb.pushDebugMarker("Dispatch indirect args", Vec3(1.0f, 1.0f, 1.0f));
				cmdb.fillBuffer(stage2Mem.m_meshlet.m_dispatchMeshIndirectArgs, 0);
				cmdb.popDebugMarker();
			}

			if(stage2Mem.m_meshlet.m_indirectDrawArgs.isValid())
			{
				cmdb.pushDebugMarker("Draw indirect args (S/W meshlet rendering)", Vec3(1.0f, 1.0f, 1.0f));
				cmdb.fillBuffer(stage2Mem.m_meshlet.m_indirectDrawArgs, 0);
				cmdb.popDebugMarker();
			}

			if(stage2Mem.m_legacy.m_mdiDrawCounts.isValid())
			{
				cmdb.pushDebugMarker("MDI counts", Vec3(1.0f, 1.0f, 1.0f));
				cmdb.fillBuffer(stage2Mem.m_legacy.m_mdiDrawCounts, 0);
				cmdb.popDebugMarker();
			}

			cmdb.pushDebugMarker("OoM readback", Vec3(1.0f, 1.0f, 1.0f));
			cmdb.fillBuffer(m_outOfMemoryReadbackBuffer, 0);
			cmdb.popDebugMarker();
		});
	}

	// 1st stage
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass(generateTempPassName("GPU vis 1st pass: %s", in.m_passesName.cstr()));

		pass.newBufferDependency(getRenderer().getGpuSceneBufferHandle(), BufferUsageBit::kStorageComputeRead);
		pass.newBufferDependency(out.m_dependency, BufferUsageBit::kStorageComputeWrite);
		pass.newBufferDependency(zeroMemDep, BufferUsageBit::kStorageComputeWrite);

		if(frustumTestData && frustumTestData->m_hzbRt.isValid())
		{
			pass.newTextureDependency(frustumTestData->m_hzbRt, TextureUsageBit::kSampledCompute);
		}

		pass.setWork([this, frustumTestData, distTestData, lodReferencePoint = in.m_lodReferencePoint, lodDistances = in.m_lodDistances,
					  technique = in.m_technique, stage1Mem, bLegacyRendering, bMeshletRendering](RenderPassWorkContext& rpass) {
			CommandBuffer& cmdb = *rpass.m_commandBuffer;

			const Bool gatherAabbIndices = stage1Mem.m_visibleAabbIndices.isValid();
			const Bool genHash = stage1Mem.m_hash.isValid();
			const Bool gatherMeshlets = stage1Mem.m_visibleMeshlets.isValid();

			if(frustumTestData)
			{
				cmdb.bindShaderProgram(m_frustumGrProgs[frustumTestData->m_hzbRt.isValid()][gatherAabbIndices][genHash][gatherMeshlets].get());
			}
			else
			{
				cmdb.bindShaderProgram(m_distGrProgs[gatherAabbIndices][genHash][gatherMeshlets].get());
			}

			BufferView aabbsBuffer;
			U32 aabbCount = 0;
			switch(technique)
			{
			case RenderingTechnique::kGBuffer:
				aabbsBuffer = GpuSceneArrays::RenderableBoundingVolumeGBuffer::getSingleton().getBufferView();
				aabbCount = GpuSceneArrays::RenderableBoundingVolumeGBuffer::getSingleton().getElementCount();
				break;
			case RenderingTechnique::kDepth:
				aabbsBuffer = GpuSceneArrays::RenderableBoundingVolumeDepth::getSingleton().getBufferView();
				aabbCount = GpuSceneArrays::RenderableBoundingVolumeDepth::getSingleton().getElementCount();
				break;
			case RenderingTechnique::kForward:
				aabbsBuffer = GpuSceneArrays::RenderableBoundingVolumeForward::getSingleton().getBufferView();
				aabbCount = GpuSceneArrays::RenderableBoundingVolumeForward::getSingleton().getElementCount();
				break;
			default:
				ANKI_ASSERT(0);
			}

			cmdb.bindStorageBuffer(ANKI_REG(t0), aabbsBuffer);
			cmdb.bindStorageBuffer(ANKI_REG(t1), GpuSceneArrays::Renderable::getSingleton().getBufferView());
			cmdb.bindStorageBuffer(ANKI_REG(t2), GpuSceneArrays::MeshLod::getSingleton().getBufferView());
			cmdb.bindStorageBuffer(ANKI_REG(t3), GpuSceneArrays::Transform::getSingleton().getBufferView());
			cmdb.bindStorageBuffer(ANKI_REG(t4), GpuSceneArrays::ParticleEmitter::getSingleton().getBufferViewSafe());

			cmdb.bindStorageBuffer(ANKI_REG(u0), stage1Mem.m_counters);

			cmdb.bindStorageBuffer(ANKI_REG(u1), (bLegacyRendering) ? stage1Mem.m_visibleRenderables : BufferView(&getRenderer().getDummyBuffer()));
			cmdb.bindStorageBuffer(ANKI_REG(u2), (bMeshletRendering) ? stage1Mem.m_visibleMeshlets : BufferView(&getRenderer().getDummyBuffer()));

			cmdb.bindStorageBuffer(ANKI_REG(u3), (bLegacyRendering) ? stage1Mem.m_renderablePrefixSums : BufferView(&getRenderer().getDummyBuffer()));
			cmdb.bindStorageBuffer(ANKI_REG(u4), (bMeshletRendering) ? stage1Mem.m_meshletPrefixSums : BufferView(&getRenderer().getDummyBuffer()));

			cmdb.bindStorageBuffer(ANKI_REG(u5), stage1Mem.m_stage2IndirectArgs);

			cmdb.bindStorageBuffer(ANKI_REG(u6), m_outOfMemoryReadbackBuffer);

			if(gatherAabbIndices)
			{
				cmdb.bindStorageBuffer(ANKI_REG(u7), stage1Mem.m_visibleAabbIndices);
			}

			if(genHash)
			{
				cmdb.bindStorageBuffer(ANKI_REG(u8), stage1Mem.m_hash);
			}

			if(frustumTestData)
			{
				FrustumGpuVisibilityUniforms* unis = allocateAndBindConstants<FrustumGpuVisibilityUniforms>(cmdb, ANKI_REG(b0));

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
					rpass.bindTexture(ANKI_REG(t5), frustumTestData->m_hzbRt);
					cmdb.bindSampler(ANKI_REG(s0), getRenderer().getSamplers().m_nearestNearestClamp.get());
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

			dispatchPPCompute(cmdb, 64, 1, aabbCount, 1);
		});
	} // end 1st stage

	// 2nd stage
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass(generateTempPassName("GPU vis 2nd pass: %s", in.m_passesName.cstr()));

		pass.newBufferDependency(out.m_dependency, BufferUsageBit::kIndirectCompute | BufferUsageBit::kStorageComputeWrite);

		if(frustumTestData && frustumTestData->m_hzbRt.isValid())
		{
			pass.newTextureDependency(frustumTestData->m_hzbRt, TextureUsageBit::kSampledCompute);
		}

		pass.setWork([this, stage1Mem, stage2Mem, bLegacyRendering, bMeshletRendering, bHwMeshletRendering, out, frustumTestData,
					  lodReferencePoint = in.m_lodReferencePoint](RenderPassWorkContext& rpass) {
			CommandBuffer& cmdb = *rpass.m_commandBuffer;

			if(bLegacyRendering)
			{
				cmdb.bindShaderProgram(m_gatherGrProg.get());

				cmdb.bindStorageBuffer(ANKI_REG(t0), GpuSceneArrays::Renderable::getSingleton().getBufferView());
				cmdb.bindStorageBuffer(ANKI_REG(t1), GpuSceneArrays::ParticleEmitter::getSingleton().getBufferViewSafe());
				cmdb.bindStorageBuffer(ANKI_REG(t2), GpuSceneArrays::MeshLod::getSingleton().getBufferView());

				cmdb.bindStorageBuffer(ANKI_REG(t3), stage1Mem.m_visibleRenderables);
				cmdb.bindStorageBuffer(ANKI_REG(t4), stage1Mem.m_counters);
				cmdb.bindStorageBuffer(ANKI_REG(t5), stage1Mem.m_renderablePrefixSums);

				UVec2* firstDrawIndirectArgAndCount =
					allocateAndBindStorageBuffer<UVec2>(cmdb, ANKI_REG(t6), out.m_legacy.m_bucketIndirectArgsRanges.getSize());
				for(U32 ibucket = 0; ibucket < out.m_legacy.m_bucketIndirectArgsRanges.getSize(); ++ibucket)
				{
					firstDrawIndirectArgAndCount->x() = out.m_legacy.m_bucketIndirectArgsRanges[ibucket].m_firstInstance;
					firstDrawIndirectArgAndCount->y() = out.m_legacy.m_bucketIndirectArgsRanges[ibucket].m_instanceCount;
					++firstDrawIndirectArgAndCount;
				}

				cmdb.bindStorageBuffer(ANKI_REG(u0), stage2Mem.m_legacy.m_instanceRateRenderables);
				cmdb.bindStorageBuffer(ANKI_REG(u1), stage2Mem.m_legacy.m_drawIndexedIndirectArgs);
				cmdb.bindStorageBuffer(ANKI_REG(u2), stage2Mem.m_legacy.m_drawIndexedIndirectArgs);

				cmdb.bindStorageBuffer(ANKI_REG(u3), stage2Mem.m_legacy.m_mdiDrawCounts);

				cmdb.bindStorageBuffer(ANKI_REG(u4), m_outOfMemoryReadbackBuffer);

				cmdb.dispatchComputeIndirect(BufferView(stage1Mem.m_stage2IndirectArgs).setRange(sizeof(DispatchIndirectArgs)));
			}

			if(bMeshletRendering)
			{
				const Bool hzbTex = frustumTestData && frustumTestData->m_hzbRt.isValid();
				const Bool passthrough = frustumTestData == nullptr;
				const Bool meshShaders = GrManager::getSingleton().getDeviceCapabilities().m_meshShaders;

				cmdb.bindShaderProgram(m_meshletGrProgs[hzbTex][passthrough][meshShaders].get());

				cmdb.bindStorageBuffer(ANKI_REG(t0), GpuSceneArrays::Renderable::getSingleton().getBufferView());
				cmdb.bindStorageBuffer(ANKI_REG(t1), GpuSceneArrays::MeshLod::getSingleton().getBufferView());
				cmdb.bindStorageBuffer(ANKI_REG(t2), GpuSceneArrays::Transform::getSingleton().getBufferView());

				cmdb.bindStorageBuffer(ANKI_REG(t3), UnifiedGeometryBuffer::getSingleton().getBufferView());

				if(hzbTex)
				{
					rpass.bindTexture(ANKI_REG(t4), frustumTestData->m_hzbRt);
					cmdb.bindSampler(ANKI_REG(s0), getRenderer().getSamplers().m_nearestNearestClamp.get());
				}

				cmdb.bindStorageBuffer(ANKI_REG(t5), stage1Mem.m_counters);
				cmdb.bindStorageBuffer(ANKI_REG(t6), stage1Mem.m_meshletPrefixSums);
				cmdb.bindStorageBuffer(ANKI_REG(t7), stage1Mem.m_visibleMeshlets);

				cmdb.bindStorageBuffer(ANKI_REG(u0), (bHwMeshletRendering) ? stage2Mem.m_meshlet.m_dispatchMeshIndirectArgs
																		   : stage2Mem.m_meshlet.m_indirectDrawArgs);
				cmdb.bindStorageBuffer(ANKI_REG(u1), stage2Mem.m_meshlet.m_meshletInstances);

				cmdb.bindStorageBuffer(ANKI_REG(u2), m_outOfMemoryReadbackBuffer);

				if(!passthrough)
				{
					class Consts
					{
					public:
						Mat4 m_viewProjectionMatrix;

						Vec3 m_cameraPos;
						U32 m_padding1;

						Vec2 m_viewportSizef;
						UVec2 m_padding2;
					} consts;
					consts.m_viewProjectionMatrix = frustumTestData->m_viewProjMat;
					consts.m_cameraPos = lodReferencePoint;
					consts.m_viewportSizef = Vec2(frustumTestData->m_finalRenderTargetSize);

					cmdb.setPushConstants(&consts, sizeof(consts));
				}

				cmdb.dispatchComputeIndirect(
					BufferView(stage1Mem.m_stage2IndirectArgs).incrementOffset(sizeof(DispatchIndirectArgs)).setRange(sizeof(DispatchIndirectArgs)));
			}
		});

	} // end 2nd stage
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
	RenderGraphBuilder& rgraph = *in.m_rgraph;

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
		out.m_visiblesBufferHandle = rgraph.importBuffer(out.m_visiblesBuffer, BufferUsageBit::kNone);

		return;
	}

	if(in.m_cpuFeedbackBuffer.isValid())
	{
		ANKI_ASSERT(in.m_cpuFeedbackBuffer.getRange() == sizeof(U32) * (objCount * 2 + 1));
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

		m_counterBufferZeroingHandle = rgraph.importBuffer(BufferView(m_counterBuffer.get()), buffInit.m_usage);

		NonGraphicsRenderPass& pass =
			rgraph.newNonGraphicsRenderPass(generateTempPassName("Non-renderables vis: Clear counter buff: %s", in.m_passesName.cstr()));

		pass.newBufferDependency(m_counterBufferZeroingHandle, BufferUsageBit::kTransferDestination);

		pass.setWork([counterBuffer = m_counterBuffer](RenderPassWorkContext& rgraph) {
			rgraph.m_commandBuffer->fillBuffer(BufferView(counterBuffer.get()), 0);
		});

		m_counterBufferOffset = 0;
	}
	else if(!firstRunInFrame)
	{
		m_counterBufferOffset += counterBufferElementSize;
	}

	// Allocate memory for the result
	out.m_visiblesBuffer = allocateTransientGpuMem((objCount + 1) * sizeof(U32));
	out.m_visiblesBufferHandle = rgraph.importBuffer(out.m_visiblesBuffer, BufferUsageBit::kNone);

	// Create the renderpass
	NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass(generateTempPassName("Non-renderables vis: %s", in.m_passesName.cstr()));

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

		const Bool needsFeedback = feedbackBuffer.isValid();

		cmdb.bindShaderProgram(m_grProgs[0][objType][needsFeedback].get());

		BufferView objBuffer;
		switch(objType)
		{
		case GpuSceneNonRenderableObjectType::kLight:
			objBuffer = GpuSceneArrays::Light::getSingleton().getBufferView();
			break;
		case GpuSceneNonRenderableObjectType::kDecal:
			objBuffer = GpuSceneArrays::Decal::getSingleton().getBufferView();
			break;
		case GpuSceneNonRenderableObjectType::kFogDensityVolume:
			objBuffer = GpuSceneArrays::FogDensityVolume::getSingleton().getBufferView();
			break;
		case GpuSceneNonRenderableObjectType::kGlobalIlluminationProbe:
			objBuffer = GpuSceneArrays::GlobalIlluminationProbe::getSingleton().getBufferView();
			break;
		case GpuSceneNonRenderableObjectType::kReflectionProbe:
			objBuffer = GpuSceneArrays::ReflectionProbe::getSingleton().getBufferView();
			break;
		default:
			ANKI_ASSERT(0);
		}
		cmdb.bindStorageBuffer(ANKI_REG(t0), objBuffer);

		GpuVisibilityNonRenderableUniforms unis;
		Array<Plane, 6> planes;
		extractClipPlanes(viewProjectionMat, planes);
		for(U32 i = 0; i < 6; ++i)
		{
			unis.m_clipPlanes[i] = Vec4(planes[i].getNormal().xyz(), planes[i].getOffset());
		}
		cmdb.setPushConstants(&unis, sizeof(unis));

		rgraph.bindStorageBuffer(ANKI_REG(u0), visibleIndicesBuffHandle);
		cmdb.bindStorageBuffer(ANKI_REG(u1), BufferView(counterBuffer.get(), counterBufferOffset, sizeof(U32) * kCountersPerDispatch));

		if(needsFeedback)
		{
			cmdb.bindStorageBuffer(ANKI_REG(u2), feedbackBuffer);
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
	inf.m_usage = BufferUsageBit::kStorageComputeWrite | BufferUsageBit::kStorageComputeRead | BufferUsageBit::kTransferDestination;
	m_counterBuffer = GrManager::getSingleton().newBuffer(inf);

	zeroBuffer(m_counterBuffer.get());

	return Error::kNone;
}

void GpuVisibilityAccelerationStructures::pupulateRenderGraph(GpuVisibilityAccelerationStructuresInput& in,
															  GpuVisibilityAccelerationStructuresOutput& out)
{
	in.validate();
	RenderGraphBuilder& rgraph = *in.m_rgraph;

#if ANKI_ASSERTIONS_ENABLED
	ANKI_ASSERT(m_lastFrameIdx != getRenderer().getFrameCount());
	m_lastFrameIdx = getRenderer().getFrameCount();
#endif

	// Allocate the transient buffers
	const U32 aabbCount = GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount();

	out.m_instancesBuffer = allocateTransientGpuMem(aabbCount * sizeof(AccelerationStructureInstance));
	out.m_someBufferHandle = rgraph.importBuffer(out.m_instancesBuffer, BufferUsageBit::kStorageComputeWrite);

	out.m_renderableIndicesBuffer = allocateTransientGpuMem((aabbCount + 1) * sizeof(U32));

	const BufferView zeroInstancesDispatchArgsBuff = allocateTransientGpuMem(sizeof(DispatchIndirectArgs));

	// Create vis pass
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass(generateTempPassName("Accel vis: %s", in.m_passesName.cstr()));

		pass.newBufferDependency(getRenderer().getGpuSceneBufferHandle(), BufferUsageBit::kStorageComputeRead);
		pass.newBufferDependency(out.m_someBufferHandle, BufferUsageBit::kStorageComputeWrite);

		pass.setWork([this, viewProjMat = in.m_viewProjectionMatrix, lodDistances = in.m_lodDistances, pointOfTest = in.m_pointOfTest,
					  testRadius = in.m_testRadius, instancesBuff = out.m_instancesBuffer, indicesBuff = out.m_renderableIndicesBuffer,
					  zeroInstancesDispatchArgsBuff](RenderPassWorkContext& rgraph) {
			CommandBuffer& cmdb = *rgraph.m_commandBuffer;

			cmdb.bindShaderProgram(m_visibilityGrProg.get());

			GpuVisibilityAccelerationStructuresUniforms unis;
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

			cmdb.bindStorageBuffer(ANKI_REG(t0), GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getBufferView());
			cmdb.bindStorageBuffer(ANKI_REG(t1), GpuSceneArrays::Renderable::getSingleton().getBufferView());
			cmdb.bindStorageBuffer(ANKI_REG(t2), GpuSceneArrays::MeshLod::getSingleton().getBufferView());
			cmdb.bindStorageBuffer(ANKI_REG(t3), GpuSceneArrays::Transform::getSingleton().getBufferView());
			cmdb.bindStorageBuffer(ANKI_REG(u0), instancesBuff);
			cmdb.bindStorageBuffer(ANKI_REG(u1), indicesBuff);
			cmdb.bindStorageBuffer(ANKI_REG(u2), BufferView(m_counterBuffer.get(), 0, sizeof(U32) * 2));
			cmdb.bindStorageBuffer(ANKI_REG(u3), zeroInstancesDispatchArgsBuff);

			const U32 aabbCount = GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount();
			dispatchPPCompute(cmdb, 64, 1, aabbCount, 1);
		});
	}

	// Zero remaining instances
	{
		NonGraphicsRenderPass& pass =
			rgraph.newNonGraphicsRenderPass(generateTempPassName("Accel vis zero remaining instances: %s", in.m_passesName.cstr()));

		pass.newBufferDependency(out.m_someBufferHandle, BufferUsageBit::kStorageComputeWrite);

		pass.setWork([this, zeroInstancesDispatchArgsBuff, instancesBuff = out.m_instancesBuffer,
					  indicesBuff = out.m_renderableIndicesBuffer](RenderPassWorkContext& rgraph) {
			CommandBuffer& cmdb = *rgraph.m_commandBuffer;

			cmdb.bindShaderProgram(m_zeroRemainingInstancesGrProg.get());

			cmdb.bindStorageBuffer(ANKI_REG(t0), indicesBuff);
			cmdb.bindStorageBuffer(ANKI_REG(U0), instancesBuff);

			cmdb.dispatchComputeIndirect(zeroInstancesDispatchArgsBuff);
		});
	}
}

} // end namespace anki
