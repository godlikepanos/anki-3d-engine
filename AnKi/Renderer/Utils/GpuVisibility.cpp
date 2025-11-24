// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Utils/GpuVisibility.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Scene/RenderStateBucket.h>
#include <AnKi/Scene/GpuSceneArray.h>
#include <AnKi/GpuMemory/GpuVisibleTransientMemoryPool.h>
#include <AnKi/GpuMemory/RebarTransientMemoryPool.h>
#include <AnKi/GpuMemory/GpuSceneBuffer.h>
#include <AnKi/Collision/Functions.h>
#include <AnKi/Shaders/Include/GpuVisibilityTypes.h>
#include <AnKi/GpuMemory/UnifiedGeometryBuffer.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Core/App.h>

namespace anki {

constexpr U32 kMaxVisibleObjects = 30 * 1024;

constexpr U32 kMaxVisiblePrimitives = 40'000'000;
constexpr U32 kMaxVisibleMeshlets = kMaxVisiblePrimitives / kMaxPrimitivesPerMeshlet;

ANKI_SVAR(GpuVisMemoryAllocated, StatCategory::kRenderer, "GPU vis mem", StatFlag::kBytes | StatFlag::kMainThreadUpdates | StatFlag::kZeroEveryFrame)
ANKI_SVAR(MaxGpuVisMemoryAllocated, StatCategory::kRenderer, "GPU vis mem: max ever used/frame", StatFlag::kBytes | StatFlag::kMainThreadUpdates)

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
	ANKI_ASSERT(meshletUserCount == 0 || (g_cvarCoreMeshletRendering || GrManager::getSingleton().getDeviceCapabilities().m_meshShaders));
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
			g_svarMaxGpuVisMemoryAllocated.set(m_maxMemUsedInFrame);
		}

		m_memUsedThisFrame += size;
	}

private:
	PtrSize m_memUsedThisFrame = 0;
	PtrSize m_maxMemUsedInFrame = 0;
	U64 m_frameIdx = kMaxU64;
};

template<typename T>
static BufferView allocateStructuredBuffer(U32 count)
{
	BufferView out = {};

	if(count > 0)
	{
		g_svarGpuVisMemoryAllocated.increment(sizeof(T) * count);
		out = GpuVisibleTransientMemoryPool::getSingleton().allocateStructuredBuffer<T>(count);

		GpuVisMemoryStats::getSingleton().informAboutAllocation(sizeof(T) * count);
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
					for(MutatorValue gatherLegacy = 0; gatherLegacy < 2; ++gatherLegacy)
					{
						if(gatherLegacy == 0 && gatherMeshlets == 0)
						{
							continue; // Not allowed
						}

						ANKI_CHECK(loadShaderProgram("ShaderBinaries/GpuVisibilityStage1.ankiprogbin",
													 {{"HZB_TEST", hzb},
													  {"DISTANCE_TEST", 0},
													  {"GATHER_AABBS", gatherAabbs},
													  {"HASH_VISIBLES", genHash},
													  {"GATHER_MESHLETS", gatherMeshlets},
													  {"GATHER_LEGACY", gatherLegacy}},
													 m_1stStageProg, m_frustumGrProgs[hzb][gatherAabbs][genHash][gatherMeshlets][gatherLegacy]));
					}
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
				for(MutatorValue gatherLegacy = 0; gatherLegacy < 2; ++gatherLegacy)
				{
					if(gatherLegacy == 0 && gatherMeshlets == 0)
					{
						continue; // Not allowed
					}

					ANKI_CHECK(loadShaderProgram("ShaderBinaries/GpuVisibilityStage1.ankiprogbin",
												 {{"HZB_TEST", 0},
												  {"DISTANCE_TEST", 1},
												  {"GATHER_AABBS", gatherAabbs},
												  {"HASH_VISIBLES", genHash},
												  {"GATHER_MESHLETS", gatherMeshlets},
												  {"GATHER_LEGACY", gatherLegacy}},
												 m_1stStageProg, m_distGrProgs[gatherAabbs][genHash][gatherMeshlets][gatherLegacy]));
				}
			}
		}
	}

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/GpuVisibilityStage2And3.ankiprogbin",
								 {{"HZB_TEST", 0}, {"PASSTHROUGH", 0}, {"MESH_SHADERS", 0}, {"STORE_MESHLETS_FAILED_HZB", 1}}, m_2ndStageProg,
								 m_gatherGrProg, "Legacy"));

	for(MutatorValue hzb = 0; hzb < 2; ++hzb)
	{
		for(MutatorValue passthrough = 0; passthrough < 2; ++passthrough)
		{
			for(MutatorValue meshShaders = 0; meshShaders < 2; ++meshShaders)
			{
				for(MutatorValue storeMeshletsFailedHzb = 0; storeMeshletsFailedHzb < 2; ++storeMeshletsFailedHzb)
				{
					ANKI_CHECK(loadShaderProgram("ShaderBinaries/GpuVisibilityStage2And3.ankiprogbin",
												 {{"HZB_TEST", hzb},
												  {"PASSTHROUGH", passthrough},
												  {"MESH_SHADERS", meshShaders},
												  {"STORE_MESHLETS_FAILED_HZB", storeMeshletsFailedHzb}},
												 m_2ndStageProg, m_meshletGrProgs[hzb][passthrough][meshShaders][storeMeshletsFailedHzb],
												 "Meshlets"));
				}
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
		out = {};
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

	Bool bStoreMeshletsFailedHzb = false;
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

		bStoreMeshletsFailedHzb = fin.m_twoPhaseOcclusionCulling;
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

		m_outOfMemoryReadbackBuffer = getRenderer().getReadbackManager().allocateStructuredBuffer<U32>(m_outOfMemoryReadback, 1);
	}

	// Get some limits
	const RenderStateBucketContainer& buckets = RenderStateBucketContainer::getSingleton();
	const U32 bucketCount = buckets.getBucketCount(in.m_technique);
	const GpuVisLimits limits = computeLimits(in.m_technique);

	const Bool bHwMeshletRendering = getRenderer().getMeshletRenderingType() == MeshletRenderingType::kMeshShaders && limits.m_maxVisibleMeshlets > 0;
	const Bool bSwMeshletRendering = getRenderer().getMeshletRenderingType() == MeshletRenderingType::kSoftware && limits.m_maxVisibleMeshlets > 0;
	const Bool bMeshletRendering = bHwMeshletRendering || bSwMeshletRendering;
	const Bool bLegacyRendering = limits.m_maxVisibleLegacyRenderables > 0;

	if(bStoreMeshletsFailedHzb)
	{
		ANKI_ASSERT(bMeshletRendering && frustumTestData->m_hzbRt.isValid());
	}

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
			allocateStructuredBuffer<GpuVisibilityVisibleRenderableDesc>(maxLimits.m_maxVisibleLegacyRenderables);
		m_persistentMemory.m_stage1.m_visibleMeshlets = allocateStructuredBuffer<GpuVisibilityVisibleMeshletDesc>(maxLimits.m_maxVisibleMeshlets);

		m_persistentMemory.m_stage2Legacy.m_perDraw = allocateStructuredBuffer<GpuScenePerDraw>(maxLimits.m_maxVisibleLegacyRenderables);
		m_persistentMemory.m_stage2Legacy.m_drawIndexedIndirectArgs =
			allocateStructuredBuffer<DrawIndexedIndirectArgs>(maxLimits.m_maxVisibleLegacyRenderables);

		m_persistentMemory.m_stage2Meshlet.m_meshletInstances = allocateStructuredBuffer<GpuSceneMeshletInstance>(maxLimits.m_maxVisibleMeshlets);

		m_persistentMemory.m_stage2Meshlet.m_meshletsFailedHzb =
			allocateStructuredBuffer<GpuVisibilityVisibleRenderableDesc>(maxLimits.m_maxVisibleMeshlets);

		m_persistentMemory.m_stage3.m_meshletInstances = allocateStructuredBuffer<GpuSceneMeshletInstance>(maxLimits.m_maxVisibleMeshlets);

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
		BufferView m_gpuVisIndirectDispatchArgs;

		BufferView m_visibleAabbIndices;
		BufferView m_hash;
	} stage1Mem;

	stage1Mem.m_counters = allocateStructuredBuffer<U32>(U32(GpuVisibilityCounter::kCount));
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
		stage1Mem.m_visibleRenderables = allocateStructuredBuffer<GpuVisibilityVisibleRenderableDesc>(limits.m_maxVisibleLegacyRenderables);
		stage1Mem.m_visibleMeshlets = allocateStructuredBuffer<GpuVisibilityVisibleMeshletDesc>(limits.m_maxVisibleMeshlets);
	}
	stage1Mem.m_renderablePrefixSums = allocateStructuredBuffer<U32>(bucketCount);
	stage1Mem.m_meshletPrefixSums = allocateStructuredBuffer<U32>(bucketCount);
	stage1Mem.m_gpuVisIndirectDispatchArgs = allocateStructuredBuffer<DispatchIndirectArgs>(U32(GpuVisibilityIndirectDispatches::kCount));

	if(in.m_gatherAabbIndices)
	{
		stage1Mem.m_visibleAabbIndices = allocateStructuredBuffer<U32>(buckets.getBucketsActiveUserCount(in.m_technique));
	}

	if(in.m_hashVisibles)
	{
		stage1Mem.m_hash = allocateStructuredBuffer<GpuVisibilityHash>(1);
	}

	// Allocate memory for stage 2
	class Stage2Mem
	{
	public:
		class
		{
		public:
			BufferView m_perDraw;
			BufferView m_drawIndexedIndirectArgs;

			BufferView m_mdiDrawCounts;
		} m_legacy;

		class
		{
		public:
			BufferView m_indirectDrawArgs;
			BufferView m_dispatchMeshIndirectArgs;

			BufferView m_meshletInstances;

			BufferView m_meshletsFailedHzb;
		} m_meshlet;
	} stage2Mem;

	if(bLegacyRendering)
	{
		if(in.m_limitMemory)
		{
			PtrSize newRange = sizeof(GpuScenePerDraw) * limits.m_maxVisibleLegacyRenderables;
			ANKI_ASSERT(newRange <= m_persistentMemory.m_stage2Legacy.m_perDraw.getRange());
			stage2Mem.m_legacy.m_perDraw = BufferView(m_persistentMemory.m_stage2Legacy.m_perDraw).setRange(newRange);

			newRange = sizeof(DrawIndexedIndirectArgs) * limits.m_maxVisibleLegacyRenderables;
			ANKI_ASSERT(newRange <= m_persistentMemory.m_stage2Legacy.m_drawIndexedIndirectArgs.getRange());
			stage2Mem.m_legacy.m_drawIndexedIndirectArgs = BufferView(m_persistentMemory.m_stage2Legacy.m_drawIndexedIndirectArgs).setRange(newRange);
		}
		else
		{
			stage2Mem.m_legacy.m_perDraw = allocateStructuredBuffer<GpuScenePerDraw>(limits.m_maxVisibleLegacyRenderables);
			stage2Mem.m_legacy.m_drawIndexedIndirectArgs = allocateStructuredBuffer<DrawIndexedIndirectArgs>(limits.m_maxVisibleLegacyRenderables);
		}

		stage2Mem.m_legacy.m_mdiDrawCounts = allocateStructuredBuffer<U32>(bucketCount);
	}

	if(bMeshletRendering)
	{
		if(bHwMeshletRendering)
		{
			stage2Mem.m_meshlet.m_dispatchMeshIndirectArgs = allocateStructuredBuffer<DispatchIndirectArgs>(bucketCount);
		}
		else
		{
			stage2Mem.m_meshlet.m_indirectDrawArgs = allocateStructuredBuffer<DrawIndirectArgs>(bucketCount);
		}

		const U32 newCount = limits.m_maxVisibleMeshlets;
		if(in.m_limitMemory)
		{
			ANKI_ASSERT(newCount * sizeof(GpuSceneMeshletInstance) <= m_persistentMemory.m_stage2Meshlet.m_meshletInstances.getRange());
			stage2Mem.m_meshlet.m_meshletInstances =
				BufferView(m_persistentMemory.m_stage2Meshlet.m_meshletInstances).setRange(newCount * sizeof(GpuSceneMeshletInstance));
		}
		else
		{
			stage2Mem.m_meshlet.m_meshletInstances = allocateStructuredBuffer<GpuSceneMeshletInstance>(newCount);
		}

		if(bStoreMeshletsFailedHzb)
		{
			const U32 newCount = limits.m_maxVisibleMeshlets;
			if(in.m_limitMemory)
			{
				ANKI_ASSERT(newCount * sizeof(GpuVisibilityVisibleMeshletDesc) <= m_persistentMemory.m_stage2Meshlet.m_meshletsFailedHzb.getRange());
				stage2Mem.m_meshlet.m_meshletsFailedHzb =
					BufferView(m_persistentMemory.m_stage2Meshlet.m_meshletsFailedHzb).setRange(newCount * sizeof(GpuVisibilityVisibleMeshletDesc));
			}
			else
			{
				stage2Mem.m_meshlet.m_meshletsFailedHzb = allocateStructuredBuffer<GpuVisibilityVisibleMeshletDesc>(newCount);
			}
		}
	}

	// Stage 3 memory
	class Stage3Mem
	{
	public:
		BufferView m_indirectDrawArgs;
		BufferView m_dispatchMeshIndirectArgs;

		BufferView m_meshletInstances;
	} stage3Mem;

	if(bStoreMeshletsFailedHzb)
	{
		if(bHwMeshletRendering)
		{
			stage3Mem.m_dispatchMeshIndirectArgs = allocateStructuredBuffer<DispatchIndirectArgs>(bucketCount);
		}
		else
		{
			stage3Mem.m_indirectDrawArgs = allocateStructuredBuffer<DrawIndirectArgs>(bucketCount);
		}

		const U32 newCount = limits.m_maxVisibleMeshlets;
		if(in.m_limitMemory)
		{
			ANKI_ASSERT(newCount * sizeof(GpuSceneMeshletInstance) <= m_persistentMemory.m_stage3.m_meshletInstances.getRange());
			stage3Mem.m_meshletInstances =
				BufferView(m_persistentMemory.m_stage3.m_meshletInstances).setRange(newCount * sizeof(GpuSceneMeshletInstance));
		}
		else
		{
			stage3Mem.m_meshletInstances = allocateStructuredBuffer<GpuSceneMeshletInstance>(newCount);
		}
	}

	// Setup output
	out.m_legacy.m_perDrawDataBuffer = stage2Mem.m_legacy.m_perDraw;
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
	if(bLegacyRendering)
	{
		out.m_legacy.m_firstPerDrawBuffer = stage1Mem.m_renderablePrefixSums;
	}
	if(bStoreMeshletsFailedHzb)
	{
		out.m_stage1And2Mem.m_meshletsFailedHzb = stage2Mem.m_meshlet.m_meshletsFailedHzb;
		out.m_stage1And2Mem.m_counters = stage1Mem.m_counters;
		out.m_stage1And2Mem.m_meshletPrefixSums = stage1Mem.m_meshletPrefixSums;
		out.m_stage1And2Mem.m_gpuVisIndirectDispatchArgs = stage1Mem.m_gpuVisIndirectDispatchArgs;

		out.m_stage3Mem.m_indirectDrawArgs = stage3Mem.m_indirectDrawArgs;
		out.m_stage3Mem.m_dispatchMeshIndirectArgs = stage3Mem.m_dispatchMeshIndirectArgs;
		out.m_stage3Mem.m_meshletInstances = stage3Mem.m_meshletInstances;
	}

	// Use one buffer as a depedency. Doesn't matter which
	out.m_dependency =
		(in.m_limitMemory) ? m_persistentMemory.m_dep : rgraph.importBuffer(stage1Mem.m_gpuVisIndirectDispatchArgs, BufferUsageBit::kNone);

	// Zero some stuff
	const BufferHandle zeroMemDep = rgraph.importBuffer(stage1Mem.m_counters, BufferUsageBit::kNone);
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass(generateTempPassName("GPU vis zero: %s", in.m_passesName.cstr()));
		pass.newBufferDependency(zeroMemDep, BufferUsageBit::kUavCompute);

		pass.setWork([stage1Mem, stage2Mem, stage3Mem](RenderPassWorkContext& rpass) {
			ANKI_TRACE_SCOPED_EVENT(GpuVisZero);
			CommandBuffer& cmdb = *rpass.m_commandBuffer;

			constexpr Bool debugZeroing = false; // For debugging purposes zero everything

#define ANKI_ZERO(buff, alwaysZero) \
	if((alwaysZero || debugZeroing) && buff.isValid()) \
	{ \
		cmdb.pushDebugMarker(#buff, Vec3(1.0f, 1.0f, 1.0f)); \
		fillBuffer(cmdb, buff, 0); \
		cmdb.popDebugMarker(); \
	}

#define ANKI_ZERO_PART(buff, alwaysZero, sizeToZero) \
	if((alwaysZero || debugZeroing) && buff.isValid()) \
	{ \
		cmdb.pushDebugMarker(#buff, Vec3(1.0f, 1.0f, 1.0f)); \
		fillBuffer(cmdb, (debugZeroing) ? buff : BufferView(buff).setRange(sizeToZero), 0); \
		cmdb.popDebugMarker(); \
	}

			ANKI_ZERO(stage1Mem.m_counters, true)
			ANKI_ZERO(stage1Mem.m_visibleRenderables, false)
			ANKI_ZERO(stage1Mem.m_visibleMeshlets, false)
			ANKI_ZERO(stage1Mem.m_renderablePrefixSums, true)
			ANKI_ZERO(stage1Mem.m_meshletPrefixSums, true)
			ANKI_ZERO(stage1Mem.m_gpuVisIndirectDispatchArgs, false)
			ANKI_ZERO_PART(stage1Mem.m_visibleAabbIndices, true, sizeof(U32))
			ANKI_ZERO(stage1Mem.m_hash, true)

			ANKI_ZERO(stage2Mem.m_legacy.m_perDraw, false)
			ANKI_ZERO(stage2Mem.m_legacy.m_drawIndexedIndirectArgs, true)
			ANKI_ZERO(stage2Mem.m_legacy.m_mdiDrawCounts, true)
			ANKI_ZERO(stage2Mem.m_meshlet.m_indirectDrawArgs, true)
			ANKI_ZERO(stage2Mem.m_meshlet.m_dispatchMeshIndirectArgs, true)
			ANKI_ZERO(stage2Mem.m_meshlet.m_meshletInstances, false)
			ANKI_ZERO(stage2Mem.m_meshlet.m_meshletsFailedHzb, false)

			ANKI_ZERO(stage3Mem.m_indirectDrawArgs, true)
			ANKI_ZERO(stage3Mem.m_dispatchMeshIndirectArgs, true)
			ANKI_ZERO(stage3Mem.m_meshletInstances, false)

#undef ANKI_ZERO
		});
	}

	// 1st stage
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass(generateTempPassName("GPU vis 1st stage: %s", in.m_passesName.cstr()));

		pass.newBufferDependency(getRenderer().getGpuSceneBufferHandle(), BufferUsageBit::kSrvCompute);
		pass.newBufferDependency(out.m_dependency, BufferUsageBit::kUavCompute);
		pass.newBufferDependency(zeroMemDep, BufferUsageBit::kUavCompute);

		if(frustumTestData && frustumTestData->m_hzbRt.isValid())
		{
			pass.newTextureDependency(frustumTestData->m_hzbRt, TextureUsageBit::kSrvCompute);
		}

		pass.setWork([this, frustumTestData, distTestData, lodReferencePoint = in.m_lodReferencePoint, lodDistances = in.m_lodDistances,
					  technique = in.m_technique, stage1Mem, bLegacyRendering, bMeshletRendering](RenderPassWorkContext& rpass) {
			ANKI_TRACE_SCOPED_EVENT(GpuVis1stStage);
			CommandBuffer& cmdb = *rpass.m_commandBuffer;

			const Bool gatherAabbIndices = stage1Mem.m_visibleAabbIndices.isValid();
			const Bool genHash = stage1Mem.m_hash.isValid();

			if(frustumTestData)
			{
				cmdb.bindShaderProgram(
					m_frustumGrProgs[frustumTestData->m_hzbRt.isValid()][gatherAabbIndices][genHash][bMeshletRendering][bLegacyRendering].get());
			}
			else
			{
				cmdb.bindShaderProgram(m_distGrProgs[gatherAabbIndices][genHash][bMeshletRendering][bLegacyRendering].get());
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

			cmdb.bindSrv(0, 0, aabbsBuffer);
			cmdb.bindSrv(1, 0, GpuSceneArrays::Renderable::getSingleton().getBufferView());
			cmdb.bindSrv(2, 0, GpuSceneArrays::MeshLod::getSingleton().getBufferView());
			cmdb.bindSrv(3, 0, GpuSceneArrays::Transform::getSingleton().getBufferView());

			cmdb.bindUav(0, 0, stage1Mem.m_counters);

			cmdb.bindUav(1, 0, (bLegacyRendering) ? stage1Mem.m_visibleRenderables : BufferView(getDummyGpuResources().m_buffer.get()));
			cmdb.bindUav(2, 0, (bMeshletRendering) ? stage1Mem.m_visibleMeshlets : BufferView(getDummyGpuResources().m_buffer.get()));

			cmdb.bindUav(3, 0, (bLegacyRendering) ? stage1Mem.m_renderablePrefixSums : BufferView(getDummyGpuResources().m_buffer.get()));
			cmdb.bindUav(4, 0, (bMeshletRendering) ? stage1Mem.m_meshletPrefixSums : BufferView(getDummyGpuResources().m_buffer.get()));

			cmdb.bindUav(5, 0, stage1Mem.m_gpuVisIndirectDispatchArgs);

			cmdb.bindUav(6, 0, m_outOfMemoryReadbackBuffer);

			if(gatherAabbIndices)
			{
				cmdb.bindUav(7, 0, stage1Mem.m_visibleAabbIndices);
			}

			if(genHash)
			{
				cmdb.bindUav(8, 0, stage1Mem.m_hash);
			}

			if(frustumTestData)
			{
				FrustumGpuVisibilityConsts* consts = allocateAndBindConstants<FrustumGpuVisibilityConsts>(cmdb, 0, 0);

				Array<Plane, 6> planes;
				extractClipPlanes(frustumTestData->m_viewProjMat, planes);
				for(U32 i = 0; i < 6; ++i)
				{
					consts->m_clipPlanes[i] = Vec4(planes[i].getNormal().xyz(), planes[i].getOffset());
				}

				ANKI_ASSERT(kMaxLodCount == 3);
				consts->m_maxLodDistances[0] = lodDistances[0];
				consts->m_maxLodDistances[1] = lodDistances[1];
				consts->m_maxLodDistances[2] = kMaxF32;
				consts->m_maxLodDistances[3] = kMaxF32;

				consts->m_lodReferencePoint = lodReferencePoint;
				consts->m_viewProjectionMat = frustumTestData->m_viewProjMat;
				consts->m_finalRenderTargetSize = Vec2(frustumTestData->m_finalRenderTargetSize);

				if(frustumTestData->m_hzbRt.isValid())
				{
					rpass.bindSrv(4, 0, frustumTestData->m_hzbRt);
					cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());
				}
			}
			else
			{
				DistanceGpuVisibilityConstants consts;
				consts.m_pointOfTest = distTestData->m_pointOfTest;
				consts.m_testRadius = distTestData->m_testRadius;

				consts.m_maxLodDistances[0] = lodDistances[0];
				consts.m_maxLodDistances[1] = lodDistances[1];
				consts.m_maxLodDistances[2] = kMaxF32;
				consts.m_maxLodDistances[3] = kMaxF32;

				consts.m_lodReferencePoint = lodReferencePoint;

				cmdb.setFastConstants(&consts, sizeof(consts));
			}

			dispatchPPCompute(cmdb, 64, 1, aabbCount, 1);
		});
	} // end 1st stage

	// 2nd stage
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass(generateTempPassName("GPU vis 2nd stage: %s", in.m_passesName.cstr()));

		pass.newBufferDependency(out.m_dependency, BufferUsageBit::kIndirectCompute | BufferUsageBit::kUavCompute);

		if(frustumTestData && frustumTestData->m_hzbRt.isValid())
		{
			pass.newTextureDependency(frustumTestData->m_hzbRt, TextureUsageBit::kSrvCompute);
		}

		pass.setWork([this, stage1Mem, stage2Mem, bLegacyRendering, bMeshletRendering, bHwMeshletRendering, out, frustumTestData,
					  lodReferencePoint = in.m_lodReferencePoint, bStoreMeshletsFailedHzb](RenderPassWorkContext& rpass) {
			ANKI_TRACE_SCOPED_EVENT(GpuVis2ndStage);
			CommandBuffer& cmdb = *rpass.m_commandBuffer;

			if(bLegacyRendering)
			{
				cmdb.bindShaderProgram(m_gatherGrProg.get());

				cmdb.bindSrv(0, 0, GpuSceneArrays::Renderable::getSingleton().getBufferView());
				cmdb.bindSrv(1, 0, GpuSceneArrays::ParticleEmitter::getSingleton().getBufferViewSafe());
				cmdb.bindSrv(2, 0, GpuSceneArrays::ParticleEmitter2::getSingleton().getBufferViewSafe());
				cmdb.bindSrv(3, 0, GpuSceneArrays::MeshLod::getSingleton().getBufferView());

				cmdb.bindSrv(4, 0, stage1Mem.m_visibleRenderables);
				cmdb.bindSrv(5, 0, stage1Mem.m_counters);
				cmdb.bindSrv(6, 0, stage1Mem.m_renderablePrefixSums);

				WeakArray<UVec2> firstDrawIndirectArgAndCount =
					allocateAndBindSrvStructuredBuffer<UVec2>(cmdb, 7, 0, out.m_legacy.m_bucketIndirectArgsRanges.getSize());
				for(U32 ibucket = 0; ibucket < out.m_legacy.m_bucketIndirectArgsRanges.getSize(); ++ibucket)
				{
					firstDrawIndirectArgAndCount[ibucket].x() = out.m_legacy.m_bucketIndirectArgsRanges[ibucket].m_firstInstance;
					firstDrawIndirectArgAndCount[ibucket].y() = out.m_legacy.m_bucketIndirectArgsRanges[ibucket].m_instanceCount;
				}

				cmdb.bindUav(0, 0, stage2Mem.m_legacy.m_perDraw);
				cmdb.bindUav(1, 0, stage2Mem.m_legacy.m_drawIndexedIndirectArgs);

				cmdb.bindUav(2, 0, stage2Mem.m_legacy.m_mdiDrawCounts);

				cmdb.bindUav(3, 0, m_outOfMemoryReadbackBuffer);

				cmdb.dispatchComputeIndirect(
					BufferView(stage1Mem.m_gpuVisIndirectDispatchArgs)
						.incrementOffset(sizeof(DispatchIndirectArgs) * U32(GpuVisibilityIndirectDispatches::k2ndStageLegacy))
						.setRange(sizeof(DispatchIndirectArgs)));
			}

			if(bMeshletRendering)
			{
				const Bool hzbTex = frustumTestData && frustumTestData->m_hzbRt.isValid();
				const Bool passthrough = frustumTestData == nullptr;
				const Bool meshShaders = GrManager::getSingleton().getDeviceCapabilities().m_meshShaders;

				cmdb.bindShaderProgram(m_meshletGrProgs[hzbTex][passthrough][meshShaders][bStoreMeshletsFailedHzb].get());

				cmdb.bindSrv(0, 0, GpuSceneArrays::Renderable::getSingleton().getBufferView());
				cmdb.bindSrv(1, 0, GpuSceneArrays::MeshLod::getSingleton().getBufferView());
				cmdb.bindSrv(2, 0, GpuSceneArrays::Transform::getSingleton().getBufferView());

				cmdb.bindSrv(3, 0, UnifiedGeometryBuffer::getSingleton().getBufferView());

				if(hzbTex)
				{
					rpass.bindSrv(4, 0, frustumTestData->m_hzbRt);
					cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());
				}

				cmdb.bindUav(0, 0, stage1Mem.m_counters);
				cmdb.bindSrv(5, 0, stage1Mem.m_meshletPrefixSums);
				cmdb.bindSrv(6, 0, stage1Mem.m_visibleMeshlets);

				cmdb.bindUav(1, 0, (bHwMeshletRendering) ? stage2Mem.m_meshlet.m_dispatchMeshIndirectArgs : stage2Mem.m_meshlet.m_indirectDrawArgs);
				cmdb.bindUav(2, 0, stage2Mem.m_meshlet.m_meshletInstances);

				cmdb.bindUav(3, 0, m_outOfMemoryReadbackBuffer);

				if(bStoreMeshletsFailedHzb)
				{
					cmdb.bindUav(4, 0, stage2Mem.m_meshlet.m_meshletsFailedHzb);
					cmdb.bindUav(5, 0, stage1Mem.m_gpuVisIndirectDispatchArgs);
				}

				if(!passthrough)
				{
					GpuVisibilityMeshletConstants consts;
					consts.m_viewProjectionMatrix = frustumTestData->m_viewProjMat;
					consts.m_cameraPos = lodReferencePoint;
					consts.m_viewportSizef = Vec2(frustumTestData->m_finalRenderTargetSize);

					cmdb.setFastConstants(&consts, sizeof(consts));
				}

				cmdb.dispatchComputeIndirect(
					BufferView(stage1Mem.m_gpuVisIndirectDispatchArgs)
						.incrementOffset(sizeof(DispatchIndirectArgs) * U32(GpuVisibilityIndirectDispatches::k2ndStageMeshlets))
						.setRange(sizeof(DispatchIndirectArgs)));
			}
		});

	} // end 2nd stage
}

void GpuVisibility::populateRenderGraphStage3(FrustumGpuVisibilityInput& in, GpuVisibilityOutput& out)
{
	if(RenderStateBucketContainer::getSingleton().getBucketsActiveUserCount(in.m_technique) == 0) [[unlikely]]
	{
		// Early exit
		out = {};
		return;
	}

	RenderGraphBuilder& rgraph = *in.m_rgraph;

	const GpuVisLimits limits = computeLimits(in.m_technique);
	const Bool bHwMeshletRendering = getRenderer().getMeshletRenderingType() == MeshletRenderingType::kMeshShaders && limits.m_maxVisibleMeshlets > 0;
	const Bool bSwMeshletRendering = getRenderer().getMeshletRenderingType() == MeshletRenderingType::kSoftware && limits.m_maxVisibleMeshlets > 0;
	const Bool bMeshletRendering = bHwMeshletRendering || bSwMeshletRendering;

	if(!bMeshletRendering)
	{
		return;
	}

	// Set the output
	if(bHwMeshletRendering)
	{
		out.m_mesh.m_dispatchMeshIndirectArgsBuffer = out.m_stage3Mem.m_dispatchMeshIndirectArgs;
	}
	else
	{
		out.m_mesh.m_drawIndirectArgs = out.m_stage3Mem.m_indirectDrawArgs;
	}
	out.m_mesh.m_meshletInstancesBuffer = out.m_stage3Mem.m_meshletInstances;

	// Create the pass
	NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass(generateTempPassName("GPU vis 3rd stage: %s", in.m_passesName.cstr()));

	pass.newBufferDependency(out.m_dependency, BufferUsageBit::kIndirectCompute | BufferUsageBit::kUavCompute);
	pass.newBufferDependency(m_persistentMemory.m_dep, BufferUsageBit::kIndirectCompute | BufferUsageBit::kUavCompute);
	pass.newTextureDependency(*in.m_hzbRt, TextureUsageBit::kSrvCompute);

	pass.setWork([this, hzbRt = *in.m_hzbRt, bHwMeshletRendering, stage1And2Mem = out.m_stage1And2Mem, stage3Mem = out.m_stage3Mem,
				  in](RenderPassWorkContext& rpass) {
		ANKI_TRACE_SCOPED_EVENT(GpuVis3rdStage);
		CommandBuffer& cmdb = *rpass.m_commandBuffer;

		const Bool hzbTex = true;
		const Bool passthrough = false;
		const Bool bStoreMeshletsFailedHzb = false;
		cmdb.bindShaderProgram(m_meshletGrProgs[hzbTex][passthrough][bHwMeshletRendering][bStoreMeshletsFailedHzb].get());

		cmdb.bindSrv(0, 0, GpuSceneArrays::Renderable::getSingleton().getBufferView());
		cmdb.bindSrv(1, 0, GpuSceneArrays::MeshLod::getSingleton().getBufferView());
		cmdb.bindSrv(2, 0, GpuSceneArrays::Transform::getSingleton().getBufferView());

		cmdb.bindSrv(3, 0, UnifiedGeometryBuffer::getSingleton().getBufferView());

		rpass.bindSrv(4, 0, hzbRt);
		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());

		cmdb.bindUav(0, 0, stage1And2Mem.m_counters);
		cmdb.bindSrv(5, 0, stage1And2Mem.m_meshletPrefixSums);
		cmdb.bindSrv(6, 0, stage1And2Mem.m_meshletsFailedHzb);

		cmdb.bindUav(1, 0, (bHwMeshletRendering) ? stage3Mem.m_dispatchMeshIndirectArgs : stage3Mem.m_indirectDrawArgs);
		cmdb.bindUav(2, 0, stage3Mem.m_meshletInstances);

		cmdb.bindUav(3, 0, m_outOfMemoryReadbackBuffer);

		GpuVisibilityMeshletConstants consts;
		consts.m_viewProjectionMatrix = in.m_viewProjectionMatrix;
		consts.m_cameraPos = in.m_lodReferencePoint;
		consts.m_viewportSizef = Vec2(in.m_viewportSize);
		cmdb.setFastConstants(&consts, sizeof(consts));

		cmdb.dispatchComputeIndirect(BufferView(stage1And2Mem.m_gpuVisIndirectDispatchArgs)
										 .incrementOffset(sizeof(DispatchIndirectArgs) * U32(GpuVisibilityIndirectDispatches::k3rdStageMeshlets))
										 .setRange(sizeof(DispatchIndirectArgs)));
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
		WeakArray<U32> count;
		out.m_visiblesBuffer = RebarTransientMemoryPool::getSingleton().allocateStructuredBuffer(1, count);
		count[0] = 0;
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

	U32 counterBufferElementSize;
	if(GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferNaturalAlignment)
	{
		counterBufferElementSize = sizeof(GpuVisibilityNonRenderablesCounters);
	}
	else
	{
		counterBufferElementSize = getAlignedRoundUp(GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferBindOffsetAlignment,
													 U32(sizeof(GpuVisibilityNonRenderablesCounters)));
	}

	if(!m_counterBuffer.isCreated() || m_counterBufferOffset + counterBufferElementSize > m_counterBuffer->getSize()) [[unlikely]]
	{
		// Counter buffer not created or not big enough, create a new one

		BufferInitInfo buffInit("GpuVisibilityNonRenderablesCounters");
		buffInit.m_size = (m_counterBuffer.isCreated()) ? m_counterBuffer->getSize() * 2 : counterBufferElementSize * kInitialCounterArraySize;
		buffInit.m_usage = BufferUsageBit::kUavCompute | BufferUsageBit::kSrvCompute | BufferUsageBit::kCopyDestination;
		m_counterBuffer = GrManager::getSingleton().newBuffer(buffInit);

		m_counterBufferZeroingHandle = rgraph.importBuffer(BufferView(m_counterBuffer.get()), buffInit.m_usage);

		NonGraphicsRenderPass& pass =
			rgraph.newNonGraphicsRenderPass(generateTempPassName("Non-renderables vis: Clear counter buff: %s", in.m_passesName.cstr()));

		pass.newBufferDependency(m_counterBufferZeroingHandle, BufferUsageBit::kUavCompute);

		pass.setWork([counterBuffer = m_counterBuffer](RenderPassWorkContext& rgraph) {
			ANKI_TRACE_SCOPED_EVENT(GpuVisNonRenderablesSetup);
			fillBuffer(*rgraph.m_commandBuffer, BufferView(counterBuffer.get()), 0);
		});

		m_counterBufferOffset = 0;
	}
	else if(!firstRunInFrame)
	{
		m_counterBufferOffset += counterBufferElementSize;
	}

	// Allocate memory for the result
	out.m_visiblesBuffer = allocateStructuredBuffer<U32>(objCount + 1);
	out.m_visiblesBufferHandle = rgraph.importBuffer(out.m_visiblesBuffer, BufferUsageBit::kNone);

	// Create the renderpass
	NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass(generateTempPassName("Non-renderables vis: %s", in.m_passesName.cstr()));

	pass.newBufferDependency(getRenderer().getGpuSceneBufferHandle(), BufferUsageBit::kSrvCompute);
	pass.newBufferDependency(out.m_visiblesBufferHandle, BufferUsageBit::kUavCompute);

	if(in.m_hzbRt)
	{
		pass.newTextureDependency(*in.m_hzbRt, TextureUsageBit::kSrvCompute);
	}

	if(m_counterBufferZeroingHandle.isValid()) [[unlikely]]
	{
		pass.newBufferDependency(m_counterBufferZeroingHandle, BufferUsageBit::kSrvCompute | BufferUsageBit::kUavCompute);
	}

	pass.setWork([this, objType = in.m_objectType, feedbackBuffer = in.m_cpuFeedbackBuffer, viewProjectionMat = in.m_viewProjectionMat,
				  visibleIndicesBuffHandle = out.m_visiblesBufferHandle, counterBuffer = m_counterBuffer, counterBufferOffset = m_counterBufferOffset,
				  objCount, hzbRt = (in.m_hzbRt) ? *in.m_hzbRt : RenderTargetHandle()](RenderPassWorkContext& rgraph) {
		ANKI_TRACE_SCOPED_EVENT(GpuVisNonRenderables);
		CommandBuffer& cmdb = *rgraph.m_commandBuffer;

		const Bool needsFeedback = feedbackBuffer.isValid();
		const Bool hasHzb = hzbRt.isValid();

		cmdb.bindShaderProgram(m_grProgs[hasHzb][objType][needsFeedback].get());

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
		cmdb.bindSrv(0, 0, objBuffer);

		GpuVisibilityNonRenderableConstants consts;
		Array<Plane, 6> planes;
		extractClipPlanes(viewProjectionMat, planes);
		for(U32 i = 0; i < 6; ++i)
		{
			consts.m_clipPlanes[i] = Vec4(planes[i].getNormal().xyz(), planes[i].getOffset());
		}
		consts.m_viewProjectionMat = viewProjectionMat;
		cmdb.setFastConstants(&consts, sizeof(consts));

		rgraph.bindUav(0, 0, visibleIndicesBuffHandle);
		cmdb.bindUav(1, 0, BufferView(counterBuffer.get(), counterBufferOffset, sizeof(GpuVisibilityNonRenderablesCounters)));

		if(needsFeedback)
		{
			cmdb.bindUav(2, 0, feedbackBuffer);
		}

		if(hasHzb)
		{
			rgraph.bindSrv(1, 0, hzbRt);
			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());
		}

		dispatchPPCompute(cmdb, 64, 1, objCount, 1);
	});
}

Error GpuVisibilityAccelerationStructures::init()
{
	ANKI_CHECK(
		loadShaderProgram("ShaderBinaries/GpuVisibilityAccelerationStructures.ankiprogbin", {}, m_visibilityProg, m_visibilityGrProg, "Visibility"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/GpuVisibilityAccelerationStructures.ankiprogbin", {}, m_visibilityProg,
								 m_zeroRemainingInstancesGrProg, "ZeroRemainingInstances"));

	BufferInitInfo inf("GpuVisibilityAccelerationStructuresCounters");
	inf.m_size = sizeof(U32) * 2;
	inf.m_usage = BufferUsageBit::kUavCompute | BufferUsageBit::kSrvCompute | BufferUsageBit::kCopyDestination;
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

	const U32 aabbCount = GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount();

	if(aabbCount == 0) [[unlikely]]
	{
		out.m_instancesBuffer = {};

		WeakArray<U32> arr2;
		out.m_renderablesBuffer = RebarTransientMemoryPool::getSingleton().allocateStructuredBuffer<U32>(1, arr2);
		arr2[0] = 0;

		WeakArray<DispatchIndirectArgs> arr3;
		out.m_buildSbtIndirectArgsBuffer = RebarTransientMemoryPool::getSingleton().allocateStructuredBuffer<DispatchIndirectArgs>(1, arr3);
		zeroMemory(arr3[0]);

		out.m_dependency = rgraph.importBuffer(out.m_renderablesBuffer, BufferUsageBit::kNone);

		return;
	}

	// Allocate the transient buffers
	out.m_instancesBuffer = allocateStructuredBuffer<AccelerationStructureInstance>(aabbCount);
	out.m_dependency = rgraph.importBuffer(out.m_instancesBuffer, BufferUsageBit::kNone);

	out.m_renderablesBuffer = allocateStructuredBuffer<LodAndRenderableIndex>(aabbCount + 1);

	const BufferView zeroInstancesAndSbtBuildDispatchArgsBuff = allocateStructuredBuffer<DispatchIndirectArgs>(2);

	out.m_buildSbtIndirectArgsBuffer = BufferView(zeroInstancesAndSbtBuildDispatchArgsBuff).incrementOffset(sizeof(DispatchIndirectArgs));

	// Create vis pass
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass(generateTempPassName("Accel vis: %s", in.m_passesName.cstr()));

		pass.newBufferDependency(getRenderer().getGpuSceneBufferHandle(), BufferUsageBit::kSrvCompute);
		pass.newBufferDependency(out.m_dependency, BufferUsageBit::kUavCompute);

		pass.setWork([this, viewProjMat = in.m_viewProjectionMatrix, lodDistances = in.m_lodDistances, pointOfTest = in.m_pointOfTest,
					  testRadius = in.m_testRadius, instancesBuff = out.m_instancesBuffer, visRenderablesBuff = out.m_renderablesBuffer,
					  zeroInstancesAndSbtBuildDispatchArgsBuff](RenderPassWorkContext& rgraph) {
			ANKI_TRACE_SCOPED_EVENT(GpuVisibilityAccelStruct);
			CommandBuffer& cmdb = *rgraph.m_commandBuffer;

			cmdb.bindShaderProgram(m_visibilityGrProg.get());

			GpuVisibilityAccelerationStructuresConstants consts;
			Array<Plane, 6> planes;
			extractClipPlanes(viewProjMat, planes);
			for(U32 i = 0; i < 6; ++i)
			{
				consts.m_clipPlanes[i] = Vec4(planes[i].getNormal().xyz(), planes[i].getOffset());
			}

			consts.m_pointOfTest = pointOfTest;
			consts.m_testRadius = testRadius;

			ANKI_ASSERT(kMaxLodCount == 3);
			consts.m_maxLodDistances[0] = lodDistances[0];
			consts.m_maxLodDistances[1] = lodDistances[1];
			consts.m_maxLodDistances[2] = kMaxF32;
			consts.m_maxLodDistances[3] = kMaxF32;

			cmdb.setFastConstants(&consts, sizeof(consts));

			cmdb.bindSrv(0, 0, GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getBufferView());
			cmdb.bindSrv(1, 0, GpuSceneArrays::Renderable::getSingleton().getBufferView());
			cmdb.bindSrv(2, 0, GpuSceneArrays::MeshLod::getSingleton().getBufferView());
			cmdb.bindSrv(3, 0, GpuSceneArrays::Transform::getSingleton().getBufferView());
			cmdb.bindUav(0, 0, instancesBuff);
			cmdb.bindUav(1, 0, visRenderablesBuff);
			cmdb.bindUav(2, 0, BufferView(m_counterBuffer.get()));
			cmdb.bindUav(3, 0, zeroInstancesAndSbtBuildDispatchArgsBuff);

			const U32 aabbCount = GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount();
			dispatchPPCompute(cmdb, 64, 1, aabbCount, 1);
		});
	}

	// Zero remaining instances
	{
		NonGraphicsRenderPass& pass =
			rgraph.newNonGraphicsRenderPass(generateTempPassName("Accel vis zero remaining instances: %s", in.m_passesName.cstr()));

		pass.newBufferDependency(out.m_dependency, BufferUsageBit::kUavCompute | BufferUsageBit::kIndirectCompute);

		pass.setWork([this, zeroInstancesAndSbtBuildDispatchArgsBuff, instancesBuff = out.m_instancesBuffer,
					  visRenderablesBuff = out.m_renderablesBuffer](RenderPassWorkContext& rgraph) {
			ANKI_TRACE_SCOPED_EVENT(GpuVisibilityAccelStructZero);
			CommandBuffer& cmdb = *rgraph.m_commandBuffer;

			cmdb.bindShaderProgram(m_zeroRemainingInstancesGrProg.get());

			cmdb.bindSrv(0, 0, visRenderablesBuff);
			cmdb.bindUav(0, 0, instancesBuff);

			cmdb.dispatchComputeIndirect(BufferView(zeroInstancesAndSbtBuildDispatchArgsBuff).setRange(sizeof(DispatchIndirectArgs)));
		});
	}
}

Error GpuVisibilityLocalLights::init()
{
	const CString fname = "ShaderBinaries/GpuVisibilityLocalLights.ankiprogbin";
	ANKI_CHECK(loadShaderProgram(fname, {}, m_visibilityProg, m_setupGrProg, "Setup"));
	ANKI_CHECK(loadShaderProgram(fname, {}, m_visibilityProg, m_countGrProg, "Count"));
	ANKI_CHECK(loadShaderProgram(fname, {}, m_visibilityProg, m_prefixSumGrProg, "PrefixSum"));
	ANKI_CHECK(loadShaderProgram(fname, {}, m_visibilityProg, m_fillGrProg, "Fill"));
	return Error::kNone;
}

void GpuVisibilityLocalLights::populateRenderGraph(GpuVisibilityLocalLightsInput& in, GpuVisibilityLocalLightsOutput& out)
{
	RenderGraphBuilder& rgraph = *in.m_rgraph;

	// Compute the bounds
	const Vec3 newCamPos = in.m_cameraPosition + in.m_lookDirection * kForwardBias;
	const Vec3 gridSize = Vec3(in.m_cellCounts) * in.m_cellSize;

	out.m_lightGridMin = newCamPos - gridSize / 2.0f;
	out.m_lightGridMax = out.m_lightGridMin + gridSize;

	const U32 cellCount = in.m_cellCounts.x() * in.m_cellCounts.y() * in.m_cellCounts.z();

	const BufferView lightIndexCountsPerCellBuff = allocateStructuredBuffer<U32>(cellCount);
	const BufferView lightIndexOffsetsPerCellBuff = allocateStructuredBuffer<U32>(cellCount);
	const BufferView lightIndexCountBuff = allocateStructuredBuffer<U32>(1);
	const BufferView lightIndexListBuff = allocateStructuredBuffer<U32>(in.m_lightIndexListSize);
	const BufferView threadgroupCountBuff = allocateStructuredBuffer<U32>(1);

	constexpr U32 kPrefixSumThreadCount = 1024; // Common for most GPUs
	constexpr U32 kPrefixSumElementCountPerThreadgroup = kPrefixSumThreadCount * 2;
	const BufferView groupWidePrefixSumsBuff =
		allocateStructuredBuffer<U32>((cellCount + kPrefixSumElementCountPerThreadgroup - 1) / kPrefixSumElementCountPerThreadgroup);

	const BufferHandle dep = rgraph.importBuffer(lightIndexCountBuff, BufferUsageBit::kNone);

	out.m_dependency = dep;
	out.m_lightIndexListBuffer = lightIndexListBuff;
	out.m_lightIndexCountsPerCellBuffer = lightIndexCountsPerCellBuff;
	out.m_lightIndexOffsetsPerCellBuffer = lightIndexOffsetsPerCellBuff;

	GpuVisibilityLocalLightsConsts consts;
	consts.m_cellSize = in.m_cellSize;
	consts.m_maxLightIndices = in.m_lightIndexListSize;
	consts.m_gridVolumeMin = out.m_lightGridMin;
	consts.m_gridVolumeSize = gridSize;
	consts.m_cellCounts = in.m_cellCounts;
	consts.m_cellCount = cellCount;

	// Setup
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass(generateTempPassName("GPU local light vis setup: %s", in.m_passesName.cstr()));

		pass.newBufferDependency(dep, BufferUsageBit::kUavCompute);

		pass.setWork([this, lightIndexCountsPerCellBuff, lightIndexCountBuff, cellCount, threadgroupCountBuff,
					  groupWidePrefixSumsBuff](RenderPassWorkContext& rgraph) {
			ANKI_TRACE_SCOPED_EVENT(GpuVisibilityLocalLightsSetup);
			CommandBuffer& cmdb = *rgraph.m_commandBuffer;

			cmdb.bindShaderProgram(m_setupGrProg.get());

			cmdb.bindUav(0, 0, lightIndexCountsPerCellBuff);
			cmdb.bindUav(1, 0, lightIndexCountBuff);
			cmdb.bindUav(2, 0, groupWidePrefixSumsBuff);
			cmdb.bindUav(3, 0, threadgroupCountBuff);

			dispatchPPCompute(cmdb, 64, 1, cellCount, 1);
		});
	}

	// Count
	const GpuSceneArrays::Light& lights = GpuSceneArrays::Light::getSingleton();
	if(lights.getElementCount())
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass(generateTempPassName("GPU local light vis count: %s", in.m_passesName.cstr()));

		pass.newBufferDependency(dep, BufferUsageBit::kUavCompute);
		pass.newBufferDependency(getRenderer().getGpuSceneBufferHandle(), BufferUsageBit::kSrvCompute);

		pass.setWork([this, lightIndexCountsPerCellBuff, lightIndexCountBuff, consts, threadgroupCountBuff,
					  groupWidePrefixSumsBuff](RenderPassWorkContext& rgraph) {
			ANKI_TRACE_SCOPED_EVENT(GpuVisibilityLocalLightsCount);

			const GpuSceneArrays::Light& lights = GpuSceneArrays::Light::getSingleton();

			CommandBuffer& cmdb = *rgraph.m_commandBuffer;

			cmdb.bindShaderProgram(m_countGrProg.get());

			cmdb.bindSrv(0, 0, lights.getBufferView());

			cmdb.bindUav(0, 0, lightIndexCountsPerCellBuff);
			cmdb.bindUav(1, 0, lightIndexCountBuff);
			cmdb.bindUav(2, 0, groupWidePrefixSumsBuff);
			cmdb.bindUav(3, 0, threadgroupCountBuff);

			cmdb.setFastConstants(&consts, sizeof(consts));

			dispatchPPCompute(cmdb, 64, 1, consts.m_cellCount, 1);
		});
	}

	// PrefixSum
	{
		NonGraphicsRenderPass& pass =
			rgraph.newNonGraphicsRenderPass(generateTempPassName("GPU local light vis prefix sum: %s", in.m_passesName.cstr()));

		pass.newBufferDependency(dep, BufferUsageBit::kUavCompute);

		pass.setWork([this, lightIndexCountsPerCellBuff, lightIndexOffsetsPerCellBuff, lightIndexCountBuff, consts,
					  groupWidePrefixSumsBuff](RenderPassWorkContext& rgraph) {
			ANKI_TRACE_SCOPED_EVENT(GpuVisibilityLocalLightsPrefixSum);
			CommandBuffer& cmdb = *rgraph.m_commandBuffer;

			cmdb.bindShaderProgram(m_prefixSumGrProg.get());

			cmdb.bindSrv(0, 0, groupWidePrefixSumsBuff);

			cmdb.bindUav(0, 0, lightIndexCountsPerCellBuff);
			cmdb.bindUav(1, 0, lightIndexOffsetsPerCellBuff);
			cmdb.bindUav(2, 0, lightIndexCountBuff);

			cmdb.setFastConstants(&consts, sizeof(consts));

			cmdb.dispatchCompute((consts.m_cellCount + kPrefixSumElementCountPerThreadgroup - 1) / kPrefixSumElementCountPerThreadgroup, 1, 1);
		});
	}

	// Fill
	if(lights.getElementCount())
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass(generateTempPassName("GPU local light vis fill: %s", in.m_passesName.cstr()));

		pass.newBufferDependency(dep, BufferUsageBit::kUavCompute);

		pass.setWork([this, lightIndexCountsPerCellBuff, lightIndexOffsetsPerCellBuff, lightIndexCountBuff, consts,
					  lightIndexListBuff](RenderPassWorkContext& rgraph) {
			ANKI_TRACE_SCOPED_EVENT(GpuVisibilityLocalLightsPrefixSum);
			const GpuSceneArrays::Light& lights = GpuSceneArrays::Light::getSingleton();

			CommandBuffer& cmdb = *rgraph.m_commandBuffer;

			cmdb.bindShaderProgram(m_fillGrProg.get());

			cmdb.bindSrv(0, 0, lights.getBufferView());
			cmdb.bindSrv(1, 0, lightIndexOffsetsPerCellBuff);

			cmdb.bindUav(0, 0, lightIndexCountBuff);
			cmdb.bindUav(1, 0, lightIndexCountsPerCellBuff);
			cmdb.bindUav(2, 0, lightIndexListBuff);

			cmdb.setFastConstants(&consts, sizeof(consts));

			dispatchPPCompute(cmdb, 64, 1, consts.m_cellCount, 1);
		});
	}
}

} // end namespace anki
