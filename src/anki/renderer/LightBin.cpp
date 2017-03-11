// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/LightBin.h>
#include <anki/scene/FrustumComponent.h>
#include <anki/scene/Visibility.h>
#include <anki/scene/MoveComponent.h>
#include <anki/scene/LightComponent.h>
#include <anki/scene/DecalComponent.h>
#include <anki/scene/ReflectionProbeComponent.h>
#include <anki/core/Trace.h>
#include <anki/util/ThreadPool.h>

namespace anki
{

/// This should be the number of light types. For now it's spots & points & probes & decals.
const U SIZE_IDX_COUNT = 4;

// Shader structs and block representations. All positions and directions in viewspace
// For documentation see the shaders

class ShaderCluster
{
public:
	U32 m_firstIdx;
};

class ShaderLight
{
public:
	Vec4 m_posRadius;
	Vec4 m_diffuseColorShadowmapId;
	Vec4 m_specularColorRadius;
};

class ShaderPointLight : public ShaderLight
{
};

class ShaderSpotLight : public ShaderLight
{
public:
	Vec4 m_lightDir;
	Vec4 m_outerCosInnerCos;
	Mat4 m_texProjectionMat; ///< Texture projection matrix
};

class ShaderProbe
{
public:
	Vec3 m_pos;
	F32 m_radiusSq;
	F32 m_cubemapIndex;
	U32 _m_pading[3];

	ShaderProbe()
	{
		// To avoid warnings
		_m_pading[0] = _m_pading[1] = _m_pading[2] = 0;
	}
};

class ShaderDecal
{
public:
	Vec4 m_diffUv;
	Vec4 m_normRoughnessUv;
	Mat4 m_texProjectionMat;
	Vec4 m_blendFactors;
};

static const U MAX_TYPED_LIGHTS_PER_CLUSTER = 16;
static const U MAX_PROBES_PER_CLUSTER = 12;
static const U MAX_DECALS_PER_CLUSTER = 8;
static const F32 INVALID_TEXTURE_INDEX = -1.0;

class ClusterLightIndex
{
public:
	ClusterLightIndex()
	{
		// Do nothing. No need to initialize
	}

	U getIndex() const
	{
		return m_index;
	}

	void setIndex(U i)
	{
		ANKI_ASSERT(i <= MAX_U16);
		m_index = i;
	}

private:
	U16 m_index;
};

static Bool operator<(const ClusterLightIndex& a, const ClusterLightIndex& b)
{
	return a.getIndex() < b.getIndex();
}

/// Store the probe radius for sorting the indices.
/// WARNING: Keep it as small as possible, that's why the members are U16
class ClusterProbeIndex
{
public:
	ClusterProbeIndex()
	{
		// Do nothing. No need to initialize
	}

	U getIndex() const
	{
		return m_index;
	}

	void setIndex(U i)
	{
		ANKI_ASSERT(i <= MAX_U16);
		m_index = i;
	}

	F32 getProbeRadius() const
	{
		return F32(m_probeRadius) / F32(MAX_U16) * F32(MAX_PROBE_RADIUS);
	}

	void setProbeRadius(F32 r)
	{
		ANKI_ASSERT(r < MAX_PROBE_RADIUS);
		m_probeRadius = r / F32(MAX_PROBE_RADIUS) * F32(MAX_U16);
	}

	Bool operator<(const ClusterProbeIndex& b) const
	{
		ANKI_ASSERT(m_probeRadius > 0.0 && b.m_probeRadius > 0.0);
		return m_probeRadius < b.m_probeRadius;
	}

private:
	static const U MAX_PROBE_RADIUS = 1000;
	U16 m_index;
	U16 m_probeRadius;
};
static_assert(sizeof(ClusterProbeIndex) == sizeof(U16) * 2, "Because we memcmp");

/// WARNING: Keep it as small as possible. The number of clusters is huge
class alignas(U32) ClusterData
{
public:
	Atomic<U8> m_pointCount;
	Atomic<U8> m_spotCount;
	Atomic<U8> m_probeCount;
	Atomic<U8> m_decalCount;

	Array<ClusterLightIndex, MAX_TYPED_LIGHTS_PER_CLUSTER> m_pointIds;
	Array<ClusterLightIndex, MAX_TYPED_LIGHTS_PER_CLUSTER> m_spotIds;
	Array<ClusterProbeIndex, MAX_PROBES_PER_CLUSTER> m_probeIds;
	Array<ClusterLightIndex, MAX_DECALS_PER_CLUSTER> m_decalIds;

	ClusterData()
	{
		// Do nothing. No need to initialize
	}

	void reset()
	{
		// Set the counts to zero and try to be faster
		*reinterpret_cast<U32*>(&m_pointCount) = 0;
	}

	void normalizeCounts()
	{
		normalize(m_pointCount, MAX_TYPED_LIGHTS_PER_CLUSTER, "point lights");
		normalize(m_spotCount, MAX_TYPED_LIGHTS_PER_CLUSTER, "spot lights");
		normalize(m_probeCount, MAX_PROBES_PER_CLUSTER, "probes");
		normalize(m_decalCount, MAX_DECALS_PER_CLUSTER, "decals");
	}

	void sortLightIds()
	{
		const U pointCount = m_pointCount.get();
		if(pointCount > 1)
		{
			std::sort(&m_pointIds[0], &m_pointIds[0] + pointCount);
		}

		const U spotCount = m_spotCount.get();
		if(spotCount > 1)
		{
			std::sort(&m_spotIds[0], &m_spotIds[0] + spotCount);
		}

		const U probeCount = m_probeCount.get();
		if(probeCount > 1)
		{
			std::sort(m_probeIds.getBegin(), m_probeIds.getBegin() + probeCount);
		}

		const U decalCount = m_decalCount.get();
		if(decalCount > 1)
		{
			std::sort(&m_decalIds[0], &m_decalIds[0] + decalCount);
		}
	}

	Bool operator==(const ClusterData& b) const
	{
		const U pointCount = m_pointCount.get();
		const U spotCount = m_spotCount.get();
		const U probeCount = m_probeCount.get();
		const U decalCount = m_decalCount.get();
		const U pointCount2 = b.m_pointCount.get();
		const U spotCount2 = b.m_spotCount.get();
		const U probeCount2 = b.m_probeCount.get();
		const U decalCount2 = b.m_decalCount.get();

		if(pointCount != pointCount2 || spotCount != spotCount2 || probeCount != probeCount2
			|| decalCount != decalCount2)
		{
			return false;
		}

		if(pointCount > 0)
		{
			if(memcmp(&m_pointIds[0], &b.m_pointIds[0], sizeof(m_pointIds[0]) * pointCount) != 0)
			{
				return false;
			}
		}

		if(spotCount > 0)
		{
			if(memcmp(&m_spotIds[0], &b.m_spotIds[0], sizeof(m_spotIds[0]) * spotCount) != 0)
			{
				return false;
			}
		}

		if(probeCount > 0)
		{
			if(memcmp(&m_probeIds[0], &b.m_probeIds[0], sizeof(b.m_probeIds[0]) * probeCount) != 0)
			{
				return false;
			}
		}

		if(decalCount > 0)
		{
			if(memcmp(&m_decalIds[0], &b.m_decalIds[0], sizeof(b.m_decalIds[0]) * decalCount) != 0)
			{
				return false;
			}
		}

		return true;
	}

private:
	static void normalize(Atomic<U8>& count, const U maxCount, CString what)
	{
		U8 a = count.get();
		count.set(a % maxCount);
		if(ANKI_UNLIKELY(a >= maxCount))
		{
			ANKI_R_LOGW("Increase cluster limit: %s", &what[0]);
		}
	}
};

/// Common data for all tasks.
class LightBinContext
{
public:
	LightBinContext(StackAllocator<U8> alloc)
		: m_alloc(alloc)
		, m_tempClusters(alloc)
	{
	}

	Mat4 m_viewMat;
	Mat4 m_viewProjMat;
	Mat4 m_camTrf;

	U32 m_maxLightIndices = 0;
	Bool m_shadowsEnabled = false;
	StackAllocator<U8> m_alloc;

	// To fill the light buffers
	WeakArray<ShaderPointLight> m_pointLights;
	WeakArray<ShaderSpotLight> m_spotLights;
	WeakArray<ShaderProbe> m_probes;
	WeakArray<ShaderDecal> m_decals;

	WeakArray<U32> m_lightIds;
	WeakArray<ShaderCluster> m_clusters;

	Atomic<U32> m_pointLightsCount = {0};
	Atomic<U32> m_spotLightsCount = {0};
	Atomic<U32> m_probeCount = {0};
	Atomic<U32> m_decalCount = {0};

	// To fill the tile buffers
	DynamicArrayAuto<ClusterData> m_tempClusters;

	// To fill the light index buffer
	Atomic<U32> m_lightIdsCount = {0};

	// Misc
	WeakArray<const VisibleNode> m_vPointLights;
	WeakArray<const VisibleNode> m_vSpotLights;
	WeakArray<const VisibleNode> m_vProbes;
	WeakArray<const VisibleNode> m_vDecals;

	Atomic<U32> m_count = {0};
	Atomic<U32> m_count2 = {0};

	TexturePtr m_diffDecalTexAtlas;
	SpinLock m_diffDecalTexAtlasMtx;
	TexturePtr m_normalRoughnessDecalTexAtlas;
	SpinLock m_normalRoughnessDecalTexAtlasMtx;

	LightBin* m_bin = nullptr;
};

/// Write the lights to the GPU buffers.
class WriteLightsTask : public ThreadPoolTask
{
public:
	LightBinContext* m_ctx = nullptr;

	Error operator()(U32 threadId, PtrSize threadsCount)
	{
		m_ctx->m_bin->binLights(threadId, threadsCount, *m_ctx);
		return ErrorCode::NONE;
	}
};

LightBin::LightBin(const GenericMemoryPoolAllocator<U8>& alloc,
	U clusterCountX,
	U clusterCountY,
	U clusterCountZ,
	ThreadPool* threadPool,
	StagingGpuMemoryManager* stagingMem)
	: m_alloc(alloc)
	, m_clusterCount(clusterCountX * clusterCountY * clusterCountZ)
	, m_threadPool(threadPool)
	, m_stagingMem(stagingMem)
	, m_barrier(threadPool->getThreadsCount())
{
	m_clusterer.init(alloc, clusterCountX, clusterCountY, clusterCountZ);
}

LightBin::~LightBin()
{
}

Error LightBin::bin(const Mat4& viewMat,
	const Mat4& projMat,
	const Mat4& viewProjMat,
	const Mat4& camTrf,
	const VisibilityTestResults& vi,
	StackAllocator<U8> frameAlloc,
	U maxLightIndices,
	Bool shadowsEnabled,
	StagingGpuMemoryToken& pointLightsToken,
	StagingGpuMemoryToken& spotLightsToken,
	StagingGpuMemoryToken* probesToken,
	StagingGpuMemoryToken& decalsToken,
	StagingGpuMemoryToken& clustersToken,
	StagingGpuMemoryToken& lightIndicesToken,
	TexturePtr& diffuseDecalTexAtlas,
	TexturePtr& normalRoughnessDecalTexAtlas)
{
	ANKI_TRACE_START_EVENT(RENDERER_LIGHT_BINNING);

	// Prepare the clusterer
	ClustererPrepareInfo pinf;
	pinf.m_viewMat = viewMat;
	pinf.m_projMat = projMat;
	pinf.m_camTrf = Transform(camTrf);
	m_clusterer.prepare(*m_threadPool, pinf);

	//
	// Quickly get the lights
	//
	U visiblePointLightsCount = vi.getCount(VisibilityGroupType::LIGHTS_POINT);
	U visibleSpotLightsCount = vi.getCount(VisibilityGroupType::LIGHTS_SPOT);
	U visibleProbeCount = vi.getCount(VisibilityGroupType::REFLECTION_PROBES);
	U visibleDecalCount = vi.getCount(VisibilityGroupType::DECALS);

	ANKI_TRACE_INC_COUNTER(RENDERER_LIGHTS, visiblePointLightsCount + visibleSpotLightsCount);

	//
	// Write the lights and tiles UBOs
	//
	Array<WriteLightsTask, ThreadPool::MAX_THREADS> tasks;
	LightBinContext ctx(frameAlloc);
	ctx.m_viewMat = viewMat;
	ctx.m_viewProjMat = viewProjMat;
	ctx.m_camTrf = camTrf;
	ctx.m_maxLightIndices = maxLightIndices;
	ctx.m_shadowsEnabled = shadowsEnabled;
	ctx.m_tempClusters.create(m_clusterCount);

	if(visiblePointLightsCount)
	{
		ShaderPointLight* data = static_cast<ShaderPointLight*>(m_stagingMem->allocateFrame(
			sizeof(ShaderPointLight) * visiblePointLightsCount, StagingGpuMemoryType::UNIFORM, pointLightsToken));

		ctx.m_pointLights = WeakArray<ShaderPointLight>(data, visiblePointLightsCount);

		ctx.m_vPointLights =
			WeakArray<const VisibleNode>(vi.getBegin(VisibilityGroupType::LIGHTS_POINT), visiblePointLightsCount);
	}
	else
	{
		pointLightsToken.markUnused();
	}

	if(visibleSpotLightsCount)
	{
		ShaderSpotLight* data = static_cast<ShaderSpotLight*>(m_stagingMem->allocateFrame(
			sizeof(ShaderSpotLight) * visibleSpotLightsCount, StagingGpuMemoryType::UNIFORM, spotLightsToken));

		ctx.m_spotLights = WeakArray<ShaderSpotLight>(data, visibleSpotLightsCount);

		ctx.m_vSpotLights =
			WeakArray<const VisibleNode>(vi.getBegin(VisibilityGroupType::LIGHTS_SPOT), visibleSpotLightsCount);
	}
	else
	{
		spotLightsToken.markUnused();
	}

	if(probesToken)
	{
		if(visibleProbeCount)
		{
			ShaderProbe* data = static_cast<ShaderProbe*>(m_stagingMem->allocateFrame(
				sizeof(ShaderProbe) * visibleProbeCount, StagingGpuMemoryType::UNIFORM, *probesToken));

			ctx.m_probes = WeakArray<ShaderProbe>(data, visibleProbeCount);

			ctx.m_vProbes =
				WeakArray<const VisibleNode>(vi.getBegin(VisibilityGroupType::REFLECTION_PROBES), visibleProbeCount);
		}
		else
		{
			probesToken->markUnused();
		}
	}

	if(visibleDecalCount)
	{
		ShaderDecal* data = static_cast<ShaderDecal*>(m_stagingMem->allocateFrame(
			sizeof(ShaderDecal) * visibleDecalCount, StagingGpuMemoryType::UNIFORM, decalsToken));

		ctx.m_decals = WeakArray<ShaderDecal>(data, visibleDecalCount);

		ctx.m_vDecals = WeakArray<const VisibleNode>(vi.getBegin(VisibilityGroupType::DECALS), visibleDecalCount);
	}
	else
	{
		decalsToken.markUnused();
	}

	ctx.m_bin = this;

	// Get mem for clusters
	ShaderCluster* data = static_cast<ShaderCluster*>(m_stagingMem->allocateFrame(
		sizeof(ShaderCluster) * m_clusterCount, StagingGpuMemoryType::STORAGE, clustersToken));

	ctx.m_clusters = WeakArray<ShaderCluster>(data, m_clusterCount);

	// Allocate light IDs
	U32* data2 = static_cast<U32*>(
		m_stagingMem->allocateFrame(maxLightIndices * sizeof(U32), StagingGpuMemoryType::STORAGE, lightIndicesToken));

	ctx.m_lightIds = WeakArray<U32>(data2, maxLightIndices);

	// Fill the first part of light ids with invalid indices. Will be used for empty clusters
	for(U i = 0; i < SIZE_IDX_COUNT; ++i)
	{
		ctx.m_lightIds[i] = 0;
	}
	ctx.m_lightIdsCount.set(SIZE_IDX_COUNT);

	// Fire the async job
	for(U i = 0; i < m_threadPool->getThreadsCount(); i++)
	{
		tasks[i].m_ctx = &ctx;

		m_threadPool->assignNewTask(i, &tasks[i]);
	}

	// Sync
	ANKI_CHECK(m_threadPool->waitForAllThreadsToFinish());

	diffuseDecalTexAtlas = ctx.m_diffDecalTexAtlas;
	normalRoughnessDecalTexAtlas = ctx.m_normalRoughnessDecalTexAtlas;

	ANKI_TRACE_STOP_EVENT(RENDERER_LIGHT_BINNING);
	return ErrorCode::NONE;
}

void LightBin::binLights(U32 threadId, PtrSize threadsCount, LightBinContext& ctx)
{
	ANKI_TRACE_START_EVENT(RENDERER_LIGHT_BINNING);
	U clusterCount = m_clusterCount;
	PtrSize start, end;

	//
	// Initialize the temp clusters
	//
	ThreadPoolTask::choseStartEnd(threadId, threadsCount, clusterCount, start, end);

	for(U i = start; i < end; ++i)
	{
		ctx.m_tempClusters[i].reset();
	}

	ANKI_TRACE_STOP_EVENT(RENDERER_LIGHT_BINNING);
	m_barrier.wait();
	ANKI_TRACE_START_EVENT(RENDERER_LIGHT_BINNING);

	//
	// Iterate lights and probes and bin them
	//
	ClustererTestResult testResult;
	m_clusterer.initTestResults(ctx.m_alloc, testResult);
	U lightCount = ctx.m_vPointLights.getSize() + ctx.m_vSpotLights.getSize();
	U totalCount = lightCount + ctx.m_vProbes.getSize() + ctx.m_vDecals.getSize();

	const U TO_BIN_COUNT = 1;
	while((start = ctx.m_count2.fetchAdd(TO_BIN_COUNT)) < totalCount)
	{
		end = min<U>(start + TO_BIN_COUNT, totalCount);

		for(U j = start; j < end; ++j)
		{
			if(j >= lightCount + ctx.m_vDecals.getSize())
			{
				U i = j - (lightCount + ctx.m_vDecals.getSize());
				SceneNode& snode = *ctx.m_vProbes[i].m_node;
				writeAndBinProbe(snode, ctx, testResult);
			}
			else if(j >= ctx.m_vPointLights.getSize() + ctx.m_vDecals.getSize())
			{
				U i = j - (ctx.m_vPointLights.getSize() + ctx.m_vDecals.getSize());

				SceneNode& snode = *ctx.m_vSpotLights[i].m_node;
				MoveComponent& move = snode.getComponent<MoveComponent>();
				LightComponent& light = snode.getComponent<LightComponent>();
				SpatialComponent& sp = snode.getComponent<SpatialComponent>();
				const FrustumComponent* frc = snode.tryGetComponent<FrustumComponent>();

				I pos = writeSpotLight(light, move, frc, ctx);
				binLight(sp, light, pos, 1, ctx, testResult);
			}
			else if(j >= ctx.m_vDecals.getSize())
			{
				U i = j - ctx.m_vDecals.getSize();

				SceneNode& snode = *ctx.m_vPointLights[i].m_node;
				MoveComponent& move = snode.getComponent<MoveComponent>();
				LightComponent& light = snode.getComponent<LightComponent>();
				SpatialComponent& sp = snode.getComponent<SpatialComponent>();

				I pos = writePointLight(light, move, ctx);
				binLight(sp, light, pos, 0, ctx, testResult);
			}
			else
			{
				U i = j;

				SceneNode& snode = *ctx.m_vDecals[i].m_node;
				writeAndBinDecal(snode, ctx, testResult);
			}
		}
	}

	//
	// Last thing, update the real clusters
	//
	ANKI_TRACE_STOP_EVENT(RENDERER_LIGHT_BINNING);
	m_barrier.wait();
	ANKI_TRACE_START_EVENT(RENDERER_LIGHT_BINNING);

	// Run per cluster
	const U CLUSTER_GROUP = 16;
	while((start = ctx.m_count.fetchAdd(CLUSTER_GROUP)) < clusterCount)
	{
		end = min<U>(start + CLUSTER_GROUP, clusterCount);

		for(U i = start; i < end; ++i)
		{
			auto& cluster = ctx.m_tempClusters[i];
			cluster.normalizeCounts();

			const U countP = cluster.m_pointCount.get();
			const U countS = cluster.m_spotCount.get();
			const U countProbe = cluster.m_probeCount.get();
			const U countDecal = cluster.m_decalCount.get();
			const U count = countP + countS + countProbe + countDecal;

			auto& c = ctx.m_clusters[i];
			c.m_firstIdx = 0; // Point to the first empty indices

			// Early exit
			if(ANKI_UNLIKELY(count == 0))
			{
				continue;
			}

			// Check if the previous cluster contains the same lights as this one and if yes then merge them. This will
			// avoid allocating new IDs (and thrashing GPU caches).
			cluster.sortLightIds();
			if(i != start)
			{
				const auto& clusterB = ctx.m_tempClusters[i - 1];

				if(cluster == clusterB)
				{
					c.m_firstIdx = ctx.m_clusters[i - 1].m_firstIdx;
					continue;
				}
			}

			U offset = ctx.m_lightIdsCount.fetchAdd(count + SIZE_IDX_COUNT);
			U initialOffset = offset;
			(void)initialOffset;

			if(offset + count + SIZE_IDX_COUNT <= ctx.m_maxLightIndices)
			{
				c.m_firstIdx = offset;

				ctx.m_lightIds[offset++] = countDecal;
				for(U i = 0; i < countDecal; ++i)
				{
					ctx.m_lightIds[offset++] = cluster.m_decalIds[i].getIndex();
				}

				ctx.m_lightIds[offset++] = countP;
				for(U i = 0; i < countP; ++i)
				{
					ctx.m_lightIds[offset++] = cluster.m_pointIds[i].getIndex();
				}

				ctx.m_lightIds[offset++] = countS;
				for(U i = 0; i < countS; ++i)
				{
					ctx.m_lightIds[offset++] = cluster.m_spotIds[i].getIndex();
				}

				ctx.m_lightIds[offset++] = countProbe;
				for(U i = 0; i < countProbe; ++i)
				{
					ctx.m_lightIds[offset++] = cluster.m_probeIds[i].getIndex();
				}

				ANKI_ASSERT(offset - initialOffset == count + SIZE_IDX_COUNT);
			}
			else
			{
				ANKI_R_LOGW("Light IDs buffer too small");
			}
		} // end for
	} // end while

	ANKI_TRACE_STOP_EVENT(RENDERER_LIGHT_BINNING);
}

I LightBin::writePointLight(const LightComponent& lightc, const MoveComponent& lightMove, LightBinContext& ctx)
{
	// Get GPU light
	I i = ctx.m_pointLightsCount.fetchAdd(1);

	ShaderPointLight& slight = ctx.m_pointLights[i];

	Vec4 pos = ctx.m_viewMat * lightMove.getWorldTransform().getOrigin().xyz1();

	slight.m_posRadius = Vec4(pos.xyz(), 1.0 / (lightc.getRadius() * lightc.getRadius()));
	slight.m_diffuseColorShadowmapId = lightc.getDiffuseColor();

	if(!lightc.getShadowEnabled() || !ctx.m_shadowsEnabled)
	{
		slight.m_diffuseColorShadowmapId.w() = INVALID_TEXTURE_INDEX;
	}
	else
	{
		slight.m_diffuseColorShadowmapId.w() = lightc.getShadowMapIndex();
	}

	slight.m_specularColorRadius = Vec4(lightc.getSpecularColor().xyz(), lightc.getRadius());

	return i;
}

I LightBin::writeSpotLight(const LightComponent& lightc,
	const MoveComponent& lightMove,
	const FrustumComponent* lightFrc,
	LightBinContext& ctx)
{
	I i = ctx.m_spotLightsCount.fetchAdd(1);

	ShaderSpotLight& light = ctx.m_spotLights[i];
	F32 shadowmapIndex = INVALID_TEXTURE_INDEX;

	if(lightc.getShadowEnabled() && ctx.m_shadowsEnabled)
	{
		// Write matrix
		static const Mat4 biasMat4(0.5, 0.0, 0.0, 0.5, 0.0, 0.5, 0.0, 0.5, 0.0, 0.0, 0.5, 0.5, 0.0, 0.0, 0.0, 1.0);
		// bias * proj_l * view_l * world_c
		light.m_texProjectionMat = biasMat4 * lightFrc->getViewProjectionMatrix() * ctx.m_camTrf;

		shadowmapIndex = F32(lightc.getShadowMapIndex());
	}

	// Pos & dist
	Vec4 pos = ctx.m_viewMat * lightMove.getWorldTransform().getOrigin().xyz1();
	light.m_posRadius = Vec4(pos.xyz(), 1.0 / (lightc.getDistance() * lightc.getDistance()));

	// Diff color and shadowmap ID now
	light.m_diffuseColorShadowmapId = Vec4(lightc.getDiffuseColor().xyz(), shadowmapIndex);

	// Spec color
	light.m_specularColorRadius = Vec4(lightc.getSpecularColor().xyz(), lightc.getDistance());

	// Light dir
	Vec3 lightDir = -lightMove.getWorldTransform().getRotation().getZAxis();
	lightDir = ctx.m_viewMat.getRotationPart() * lightDir;
	light.m_lightDir = Vec4(lightDir, 0.0);

	// Angles
	light.m_outerCosInnerCos = Vec4(lightc.getOuterAngleCos(), lightc.getInnerAngleCos(), 1.0, 1.0);

	return i;
}

void LightBin::binLight(const SpatialComponent& sp,
	const LightComponent& lightc,
	U pos,
	U lightType,
	LightBinContext& ctx,
	ClustererTestResult& testResult) const
{
	if(lightc.getLightComponentType() == LightComponentType::SPOT)
	{
		const FrustumComponent& frc = lightc.getSceneNode().getComponent<FrustumComponent>();
		ANKI_ASSERT(frc.getFrustum().getType() == FrustumType::PERSPECTIVE);
		m_clusterer.binPerspectiveFrustum(
			static_cast<const PerspectiveFrustum&>(frc.getFrustum()), sp.getAabb(), testResult);
	}
	else
	{
		m_clusterer.bin(sp.getSpatialCollisionShape(), sp.getAabb(), testResult);
	}

	// Bin to the correct tiles
	auto it = testResult.getClustersBegin();
	auto end = testResult.getClustersEnd();
	for(; it != end; ++it)
	{
		U x = (*it).x();
		U y = (*it).y();
		U z = (*it).z();

		U i = m_clusterer.getClusterCountX() * (z * m_clusterer.getClusterCountY() + y) + x;

		auto& cluster = ctx.m_tempClusters[i];

		switch(lightType)
		{
		case 0:
			i = cluster.m_pointCount.fetchAdd(1) % MAX_TYPED_LIGHTS_PER_CLUSTER;
			cluster.m_pointIds[i].setIndex(pos);
			break;
		case 1:
			i = cluster.m_spotCount.fetchAdd(1) % MAX_TYPED_LIGHTS_PER_CLUSTER;
			cluster.m_spotIds[i].setIndex(pos);
			break;
		default:
			ANKI_ASSERT(0);
		}
	}
}

void LightBin::writeAndBinProbe(const SceneNode& node, LightBinContext& ctx, ClustererTestResult& testResult)
{
	const ReflectionProbeComponent& reflc = node.getComponent<ReflectionProbeComponent>();
	const SpatialComponent& sp = node.getComponent<SpatialComponent>();

	// Write it
	ShaderProbe probe;
	probe.m_pos = reflc.getPosition().xyz();
	probe.m_radiusSq = reflc.getRadius() * reflc.getRadius();
	probe.m_cubemapIndex = reflc.getTextureArrayIndex();

	U idx = ctx.m_probeCount.fetchAdd(1);
	ctx.m_probes[idx] = probe;

	// Bin it
	m_clusterer.bin(sp.getSpatialCollisionShape(), sp.getAabb(), testResult);

	auto it = testResult.getClustersBegin();
	auto end = testResult.getClustersEnd();
	for(; it != end; ++it)
	{
		U x = (*it).x();
		U y = (*it).y();
		U z = (*it).z();

		U i = m_clusterer.getClusterCountX() * (z * m_clusterer.getClusterCountY() + y) + x;

		auto& cluster = ctx.m_tempClusters[i];

		i = cluster.m_probeCount.fetchAdd(1) % MAX_PROBES_PER_CLUSTER;
		cluster.m_probeIds[i].setIndex(idx);
		cluster.m_probeIds[i].setProbeRadius(reflc.getRadius());
	}
}

void LightBin::writeAndBinDecal(const SceneNode& node, LightBinContext& ctx, ClustererTestResult& testResult)
{
	const DecalComponent& decalc = node.getComponent<DecalComponent>();
	const SpatialComponent& sp = node.getComponent<SpatialComponent>();

	I idx = ctx.m_decalCount.fetchAdd(1);
	ShaderDecal& decal = ctx.m_decals[idx];

	TexturePtr atlas;
	Vec4 uv;
	F32 blendFactor;
	decalc.getDiffuseAtlasInfo(uv, atlas, blendFactor);
	decal.m_diffUv = Vec4(uv.x(), uv.y(), uv.z() - uv.x(), uv.w() - uv.y());
	decal.m_blendFactors[0] = blendFactor;

	{
		LockGuard<SpinLock> lock(ctx.m_diffDecalTexAtlasMtx);
		if(ctx.m_diffDecalTexAtlas && ctx.m_diffDecalTexAtlas != atlas)
		{
			ANKI_R_LOGF("All decals should have the same tex atlas");
		}

		ctx.m_diffDecalTexAtlas = atlas;
	}

	decalc.getNormalRoughnessAtlasInfo(uv, atlas, blendFactor);
	decal.m_normRoughnessUv = Vec4(uv.x(), uv.y(), uv.z() - uv.x(), uv.w() - uv.y());
	decal.m_blendFactors[1] = blendFactor;

	if(atlas)
	{
		LockGuard<SpinLock> lock(ctx.m_normalRoughnessDecalTexAtlasMtx);
		if(ctx.m_normalRoughnessDecalTexAtlas && ctx.m_normalRoughnessDecalTexAtlas != atlas)
		{
			ANKI_R_LOGF("All decals should have the same tex atlas");
		}

		ctx.m_normalRoughnessDecalTexAtlas = atlas;
	}

	// bias * proj_l * view_l * world_c
	decal.m_texProjectionMat = decalc.getBiasProjectionViewMatrix() * ctx.m_camTrf;

	// Bin it
	m_clusterer.bin(sp.getSpatialCollisionShape(), sp.getAabb(), testResult);

	auto it = testResult.getClustersBegin();
	auto end = testResult.getClustersEnd();
	for(; it != end; ++it)
	{
		U x = (*it).x();
		U y = (*it).y();
		U z = (*it).z();

		U i = m_clusterer.getClusterCountX() * (z * m_clusterer.getClusterCountY() + y) + x;

		auto& cluster = ctx.m_tempClusters[i];

		i = cluster.m_decalCount.fetchAdd(1) % MAX_DECALS_PER_CLUSTER;
		cluster.m_decalIds[i].setIndex(idx);
	}
}

} // end namespace anki
