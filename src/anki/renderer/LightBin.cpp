// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/LightBin.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/core/Trace.h>
#include <anki/util/ThreadPool.h>
#include <anki/collision/Sphere.h>
#include <anki/collision/Frustum.h>

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

class ShaderPointLight
{
public:
	Vec4 m_posRadius;
	Vec4 m_diffuseColorTileSize;
	Vec4 m_radiusPad3;
	UVec4 m_atlasTilesPad2;
};

class ShaderSpotLight
{
public:
	Vec4 m_posRadius;
	Vec4 m_diffuseColorShadowmapId;
	Vec4 m_lightDirRadius;
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
		ANKI_ASSERT(m_probeRadius > 0 && b.m_probeRadius > 0);
		return (m_probeRadius != b.m_probeRadius) ? (m_probeRadius > b.m_probeRadius) : (m_index < b.m_index);
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
	WeakArray<const PointLightQueueElement> m_vPointLights;
	WeakArray<const SpotLightQueueElement> m_vSpotLights;
	WeakArray<const ReflectionProbeQueueElement> m_vProbes;
	WeakArray<const DecalQueueElement> m_vDecals;

	Atomic<U32> m_count = {0};
	Atomic<U32> m_count2 = {0};

	TextureViewPtr m_diffDecalTexAtlas;
	SpinLock m_diffDecalTexAtlasMtx;
	TextureViewPtr m_specularRoughnessDecalTexAtlas;
	SpinLock m_specularRoughnessDecalTexAtlasMtx;

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
		return Error::NONE;
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
	, m_barrier(threadPool->getThreadCount())
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
	const RenderQueue& rqueue,
	StackAllocator<U8> frameAlloc,
	U maxLightIndices,
	Bool shadowsEnabled,
	LightBinOut& out)
{
	ANKI_TRACE_SCOPED_EVENT(RENDERER_LIGHT_BINNING);

	// Prepare the clusterer
	ClustererPrepareInfo pinf;
	pinf.m_viewMat = viewMat;
	pinf.m_projMat = projMat;
	pinf.m_viewProjMat = viewProjMat;
	pinf.m_camTrf = Transform(camTrf);
	pinf.m_near = rqueue.m_cameraNear;
	pinf.m_far = rqueue.m_cameraFar;
	m_clusterer.prepare(*m_threadPool, pinf);

	//
	// Quickly get the lights
	//
	const U visiblePointLightsCount = rqueue.m_pointLights.getSize();
	const U visibleSpotLightsCount = rqueue.m_spotLights.getSize();
	const U visibleProbeCount = rqueue.m_reflectionProbes.getSize();
	const U visibleDecalCount = rqueue.m_decals.getSize();

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
			sizeof(ShaderPointLight) * visiblePointLightsCount, StagingGpuMemoryType::UNIFORM, out.m_pointLightsToken));

		ctx.m_pointLights = WeakArray<ShaderPointLight>(data, visiblePointLightsCount);

		ctx.m_vPointLights =
			WeakArray<const PointLightQueueElement>(rqueue.m_pointLights.getBegin(), visiblePointLightsCount);
	}
	else
	{
		out.m_pointLightsToken.markUnused();
	}

	if(visibleSpotLightsCount)
	{
		ShaderSpotLight* data = static_cast<ShaderSpotLight*>(m_stagingMem->allocateFrame(
			sizeof(ShaderSpotLight) * visibleSpotLightsCount, StagingGpuMemoryType::UNIFORM, out.m_spotLightsToken));

		ctx.m_spotLights = WeakArray<ShaderSpotLight>(data, visibleSpotLightsCount);

		ctx.m_vSpotLights =
			WeakArray<const SpotLightQueueElement>(rqueue.m_spotLights.getBegin(), visibleSpotLightsCount);
	}
	else
	{
		out.m_spotLightsToken.markUnused();
	}

	if(visibleProbeCount)
	{
		ShaderProbe* data = static_cast<ShaderProbe*>(m_stagingMem->allocateFrame(
			sizeof(ShaderProbe) * visibleProbeCount, StagingGpuMemoryType::UNIFORM, out.m_probesToken));

		ctx.m_probes = WeakArray<ShaderProbe>(data, visibleProbeCount);

		ctx.m_vProbes =
			WeakArray<const ReflectionProbeQueueElement>(rqueue.m_reflectionProbes.getBegin(), visibleProbeCount);
	}
	else
	{
		out.m_probesToken.markUnused();
	}

	if(visibleDecalCount)
	{
		ShaderDecal* data = static_cast<ShaderDecal*>(m_stagingMem->allocateFrame(
			sizeof(ShaderDecal) * visibleDecalCount, StagingGpuMemoryType::UNIFORM, out.m_decalsToken));

		ctx.m_decals = WeakArray<ShaderDecal>(data, visibleDecalCount);

		ctx.m_vDecals = WeakArray<const DecalQueueElement>(rqueue.m_decals.getBegin(), visibleDecalCount);
	}
	else
	{
		out.m_decalsToken.markUnused();
	}

	ctx.m_bin = this;

	// Get mem for clusters
	ShaderCluster* data = static_cast<ShaderCluster*>(m_stagingMem->allocateFrame(
		sizeof(ShaderCluster) * m_clusterCount, StagingGpuMemoryType::STORAGE, out.m_clustersToken));

	ctx.m_clusters = WeakArray<ShaderCluster>(data, m_clusterCount);

	// Allocate light IDs
	U32* data2 = static_cast<U32*>(m_stagingMem->allocateFrame(
		maxLightIndices * sizeof(U32), StagingGpuMemoryType::STORAGE, out.m_lightIndicesToken));

	ctx.m_lightIds = WeakArray<U32>(data2, maxLightIndices);

	// Fill the first part of light ids with invalid indices. Will be used for empty clusters
	for(U i = 0; i < SIZE_IDX_COUNT; ++i)
	{
		ctx.m_lightIds[i] = 0;
	}
	ctx.m_lightIdsCount.set(SIZE_IDX_COUNT);

	// Fire the async job
	for(U i = 0; i < m_threadPool->getThreadCount(); i++)
	{
		tasks[i].m_ctx = &ctx;

		m_threadPool->assignNewTask(i, &tasks[i]);
	}

	// Sync
	ANKI_CHECK(m_threadPool->waitForAllThreadsToFinish());

	out.m_diffDecalTexView = ctx.m_diffDecalTexAtlas;
	out.m_specularRoughnessDecalTexView = ctx.m_specularRoughnessDecalTexAtlas;

	return Error::NONE;
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
				writeAndBinProbe(ctx.m_vProbes[i], ctx, testResult);
			}
			else if(j >= ctx.m_vPointLights.getSize() + ctx.m_vDecals.getSize())
			{
				U i = j - (ctx.m_vPointLights.getSize() + ctx.m_vDecals.getSize());
				writeAndBinSpotLight(ctx.m_vSpotLights[i], ctx, testResult);
			}
			else if(j >= ctx.m_vDecals.getSize())
			{
				U i = j - ctx.m_vDecals.getSize();
				writeAndBinPointLight(ctx.m_vPointLights[i], ctx, testResult);
			}
			else
			{
				U i = j;
				writeAndBinDecal(ctx.m_vDecals[i], ctx, testResult);
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

void LightBin::writeAndBinPointLight(
	const PointLightQueueElement& lightEl, LightBinContext& ctx, ClustererTestResult& testResult)
{
	// Get GPU light
	I idx = ctx.m_pointLightsCount.fetchAdd(1);

	ShaderPointLight& slight = ctx.m_pointLights[idx];

	slight.m_posRadius = Vec4(lightEl.m_worldPosition.xyz(), 1.0f / (lightEl.m_radius * lightEl.m_radius));
	slight.m_diffuseColorTileSize = lightEl.m_diffuseColor.xyz0();

	if(lightEl.m_shadowRenderQueues[0] == nullptr || !ctx.m_shadowsEnabled)
	{
		slight.m_diffuseColorTileSize.w() = INVALID_TEXTURE_INDEX;
	}
	else
	{
		slight.m_diffuseColorTileSize.w() = lightEl.m_atlasTileSize;
		slight.m_atlasTilesPad2 = UVec4(lightEl.m_atlasTiles.x(), lightEl.m_atlasTiles.y(), 0, 0);
	}

	slight.m_radiusPad3 = Vec4(lightEl.m_radius);

	// Now bin it
	Sphere sphere(lightEl.m_worldPosition.xyz0(), lightEl.m_radius);
	Aabb box;
	sphere.computeAabb(box);
	m_clusterer.bin(sphere, box, testResult);

	auto it = testResult.getClustersBegin();
	auto end = testResult.getClustersEnd();
	for(; it != end; ++it)
	{
		U x = (*it).x();
		U y = (*it).y();
		U z = (*it).z();

		U i = m_clusterer.getClusterCountX() * (z * m_clusterer.getClusterCountY() + y) + x;

		auto& cluster = ctx.m_tempClusters[i];

		i = cluster.m_pointCount.fetchAdd(1) % MAX_TYPED_LIGHTS_PER_CLUSTER;
		cluster.m_pointIds[i].setIndex(idx);
	}
}

void LightBin::writeAndBinSpotLight(
	const SpotLightQueueElement& lightEl, LightBinContext& ctx, ClustererTestResult& testResult)
{
	I idx = ctx.m_spotLightsCount.fetchAdd(1);

	ShaderSpotLight& light = ctx.m_spotLights[idx];
	F32 shadowmapIndex = INVALID_TEXTURE_INDEX;

	if(lightEl.hasShadow() && ctx.m_shadowsEnabled)
	{
		// bias * proj_l * view_l
		light.m_texProjectionMat = lightEl.m_textureMatrix;

		shadowmapIndex = 1.0f; // Just set a value
	}

	// Pos & dist
	light.m_posRadius =
		Vec4(lightEl.m_worldTransform.getTranslationPart().xyz(), 1.0f / (lightEl.m_distance * lightEl.m_distance));

	// Diff color and shadowmap ID now
	light.m_diffuseColorShadowmapId = Vec4(lightEl.m_diffuseColor, shadowmapIndex);

	// Light dir & radius
	Vec3 lightDir = -lightEl.m_worldTransform.getRotationPart().getZAxis();
	light.m_lightDirRadius = Vec4(lightDir, lightEl.m_distance);

	// Angles
	light.m_outerCosInnerCos = Vec4(cos(lightEl.m_outerAngle / 2.0f), cos(lightEl.m_innerAngle / 2.0f), 1.0f, 1.0f);

	// Bin lights
	PerspectiveFrustum shape(lightEl.m_outerAngle, lightEl.m_outerAngle, 0.01f, lightEl.m_distance);
	shape.transform(Transform(lightEl.m_worldTransform));
	Aabb box;
	shape.computeAabb(box);
	m_clusterer.binPerspectiveFrustum(shape, box, testResult);

	auto it = testResult.getClustersBegin();
	auto end = testResult.getClustersEnd();
	for(; it != end; ++it)
	{
		U x = (*it).x();
		U y = (*it).y();
		U z = (*it).z();

		U i = m_clusterer.getClusterCountX() * (z * m_clusterer.getClusterCountY() + y) + x;

		auto& cluster = ctx.m_tempClusters[i];

		i = cluster.m_spotCount.fetchAdd(1) % MAX_TYPED_LIGHTS_PER_CLUSTER;
		cluster.m_spotIds[i].setIndex(idx);
	}
}

void LightBin::writeAndBinProbe(
	const ReflectionProbeQueueElement& probeEl, LightBinContext& ctx, ClustererTestResult& testResult)
{
	// Write it
	ShaderProbe probe;
	probe.m_pos = probeEl.m_worldPosition;
	probe.m_radiusSq = probeEl.m_radius * probeEl.m_radius;
	probe.m_cubemapIndex = probeEl.m_textureArrayIndex;

	U idx = ctx.m_probeCount.fetchAdd(1);
	ctx.m_probes[idx] = probe;

	// Bin it
	Sphere sphere(probeEl.m_worldPosition.xyz0(), probeEl.m_radius);
	Aabb box;
	sphere.computeAabb(box);
	m_clusterer.bin(sphere, box, testResult);

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
		cluster.m_probeIds[i].setProbeRadius(probeEl.m_radius);
	}
}

void LightBin::writeAndBinDecal(const DecalQueueElement& decalEl, LightBinContext& ctx, ClustererTestResult& testResult)
{
	I idx = ctx.m_decalCount.fetchAdd(1);
	ShaderDecal& decal = ctx.m_decals[idx];

	TextureViewPtr atlas(const_cast<TextureView*>(decalEl.m_diffuseAtlas));
	Vec4 uv = decalEl.m_diffuseAtlasUv;
	decal.m_diffUv = Vec4(uv.x(), uv.y(), uv.z() - uv.x(), uv.w() - uv.y());
	decal.m_blendFactors[0] = decalEl.m_diffuseAtlasBlendFactor;

	{
		LockGuard<SpinLock> lock(ctx.m_diffDecalTexAtlasMtx);
		if(ctx.m_diffDecalTexAtlas && ctx.m_diffDecalTexAtlas != atlas)
		{
			ANKI_R_LOGF("All decals should have the same tex atlas");
		}

		ctx.m_diffDecalTexAtlas = atlas;
	}

	atlas.reset(const_cast<TextureView*>(decalEl.m_specularRoughnessAtlas));
	uv = decalEl.m_specularRoughnessAtlasUv;
	decal.m_normRoughnessUv = Vec4(uv.x(), uv.y(), uv.z() - uv.x(), uv.w() - uv.y());
	decal.m_blendFactors[1] = decalEl.m_specularRoughnessAtlasBlendFactor;

	if(atlas)
	{
		LockGuard<SpinLock> lock(ctx.m_specularRoughnessDecalTexAtlasMtx);
		if(ctx.m_specularRoughnessDecalTexAtlas && ctx.m_specularRoughnessDecalTexAtlas != atlas)
		{
			ANKI_R_LOGF("All decals should have the same tex atlas");
		}

		ctx.m_specularRoughnessDecalTexAtlas = atlas;
	}

	// bias * proj_l * view_
	decal.m_texProjectionMat = decalEl.m_textureMatrix;

	// Bin it
	Obb obb(decalEl.m_obbCenter.xyz0(), Mat3x4(decalEl.m_obbRotation), decalEl.m_obbExtend.xyz0());
	Aabb box;
	obb.computeAabb(box);
	m_clusterer.bin(obb, box, testResult);

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
