// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/LightBin.h>
#include <anki/scene/FrustumComponent.h>
#include <anki/scene/Visibility.h>
#include <anki/scene/MoveComponent.h>
#include <anki/scene/LightComponent.h>
#include <anki/scene/ReflectionProbeComponent.h>
#include <anki/core/Trace.h>
#include <anki/util/ThreadPool.h>

namespace anki
{

//==============================================================================
// Misc                                                                        =
//==============================================================================

// Shader structs and block representations. All positions and directions in
// viewspace
// For documentation see the shaders

struct ShaderCluster
{
	/// If m_combo = 0xFFFF?CAB then FFFF is the light index offset, A the
	/// number of point lights and B the number of spot lights, C the number
	/// of probes
	U32 m_combo;
};

struct ShaderLight
{
	Vec4 m_posRadius;
	Vec4 m_diffuseColorShadowmapId;
	Vec4 m_specularColorTexId;
};

struct ShaderPointLight : ShaderLight
{
};

struct ShaderSpotLight : ShaderLight
{
	Vec4 m_lightDir;
	Vec4 m_outerCosInnerCos;
	Mat4 m_texProjectionMat; ///< Texture projection matrix
};

struct ShaderProbe
{
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

static const U MAX_TYPED_LIGHTS_PER_CLUSTER = 16;
static const U MAX_PROBES_PER_CLUSTER = 12;
static const F32 INVALID_TEXTURE_INDEX = 128.0;

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

private:
	static const U MAX_PROBE_RADIUS = 1000;
	U16 m_index;
	U16 m_probeRadius;
};
static_assert(
	sizeof(ClusterProbeIndex) == sizeof(U16) * 2, "Because we memcmp");

/// WARNING: Keep it as small as possible. The number of clusters is huge
class alignas(U32) ClusterData
{
public:
	Atomic<U8> m_pointCount;
	Atomic<U8> m_spotCount;
	Atomic<U8> m_probeCount;

	Array<ClusterLightIndex, MAX_TYPED_LIGHTS_PER_CLUSTER> m_pointIds;
	Array<ClusterLightIndex, MAX_TYPED_LIGHTS_PER_CLUSTER> m_spotIds;
	Array<ClusterProbeIndex, MAX_PROBES_PER_CLUSTER> m_probeIds;

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
			std::sort(m_probeIds.getBegin(),
				m_probeIds.getBegin() + probeCount,
				[](const ClusterProbeIndex& a, const ClusterProbeIndex& b) {
					ANKI_ASSERT(
						a.getProbeRadius() > 0.0 && b.getProbeRadius() > 0.0);
					return a.getProbeRadius() < b.getProbeRadius();
				});
		}
	}

	Bool operator==(const ClusterData& b) const
	{
		const U pointCount = m_pointCount.get();
		const U spotCount = m_spotCount.get();
		const U probeCount = m_probeCount.get();
		const U pointCount2 = b.m_pointCount.get();
		const U spotCount2 = b.m_spotCount.get();
		const U probeCount2 = b.m_probeCount.get();

		if(pointCount != pointCount2 || spotCount != spotCount2
			|| probeCount != probeCount2)
		{
			return false;
		}

		if(pointCount > 0)
		{
			if(memcmp(&m_pointIds[0],
				   &b.m_pointIds[0],
				   sizeof(m_pointIds[0]) * pointCount)
				!= 0)
			{
				return false;
			}
		}

		if(spotCount > 0)
		{
			if(memcmp(&m_spotIds[0],
				   &b.m_spotIds[0],
				   sizeof(m_spotIds[0]) * spotCount)
				!= 0)
			{
				return false;
			}
		}

		if(probeCount > 0)
		{
			if(memcmp(&m_probeIds[0],
				   &b.m_probeIds[0],
				   sizeof(b.m_probeIds[0]) * probeCount)
				!= 0)
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
			ANKI_LOGW("Increase cluster limit: %s", &what[0]);
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

	FrustumComponent* m_frc = nullptr;
	U32 m_maxLightIndices = 0;
	Bool m_shadowsEnabled = false;
	StackAllocator<U8> m_alloc;

	// To fill the light buffers
	WeakArray<ShaderPointLight> m_pointLights;
	WeakArray<ShaderSpotLight> m_spotLights;
	WeakArray<ShaderProbe> m_probes;

	WeakArray<U32> m_lightIds;
	WeakArray<ShaderCluster> m_clusters;

	Atomic<U32> m_pointLightsCount = {0};
	Atomic<U32> m_spotLightsCount = {0};
	Atomic<U32> m_probeCount = {0};

	// To fill the tile buffers
	DynamicArrayAuto<ClusterData> m_tempClusters;

	// To fill the light index buffer
	Atomic<U32> m_lightIdsCount = {0};

	// Misc
	WeakArray<VisibleNode> m_vPointLights;
	WeakArray<VisibleNode> m_vSpotLights;
	WeakArray<VisibleNode> m_vProbes;

	Atomic<U32> m_count = {0};
	Atomic<U32> m_count2 = {0};

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

//==============================================================================
// LightBin                                                                    =
//==============================================================================

//==============================================================================
LightBin::LightBin(const GenericMemoryPoolAllocator<U8>& alloc,
	U clusterCountX,
	U clusterCountY,
	U clusterCountZ,
	ThreadPool* threadPool,
	GrManager* gr)
	: m_alloc(alloc)
	, m_clusterCount(clusterCountX * clusterCountY * clusterCountZ)
	, m_threadPool(threadPool)
	, m_gr(gr)
	, m_barrier(threadPool->getThreadsCount())
{
	m_clusterer.init(alloc, clusterCountX, clusterCountY, clusterCountZ);
}

//==============================================================================
LightBin::~LightBin()
{
}

//==============================================================================
Error LightBin::bin(FrustumComponent& frc,
	StackAllocator<U8> frameAlloc,
	U maxLightIndices,
	Bool shadowsEnabled,
	TransientMemoryToken& pointLightsToken,
	TransientMemoryToken& spotLightsToken,
	TransientMemoryToken* probesToken,
	TransientMemoryToken& clustersToken,
	TransientMemoryToken& lightIndicesToken)
{
	ANKI_TRACE_START_EVENT(RENDER_LIGHT_BINNING);

	// Prepare the clusterer
	m_clusterer.prepare(*m_threadPool, frc);

	VisibilityTestResults& vi = frc.getVisibilityTestResults();

	//
	// Quickly get the lights
	//
	U visiblePointLightsCount = vi.getCount(VisibilityGroupType::LIGHTS_POINT);
	U visibleSpotLightsCount = vi.getCount(VisibilityGroupType::LIGHTS_SPOT);
	U visibleProbeCount = vi.getCount(VisibilityGroupType::REFLECTION_PROBES);

	ANKI_TRACE_INC_COUNTER(
		RENDERER_LIGHTS, visiblePointLightsCount + visibleSpotLightsCount);

	//
	// Write the lights and tiles UBOs
	//
	Array<WriteLightsTask, ThreadPool::MAX_THREADS> tasks;
	LightBinContext ctx(frameAlloc);
	ctx.m_frc = &frc;
	ctx.m_maxLightIndices = maxLightIndices;
	ctx.m_shadowsEnabled = shadowsEnabled;
	ctx.m_tempClusters.create(m_clusterCount);

	if(visiblePointLightsCount)
	{
		ShaderPointLight* data =
			static_cast<ShaderPointLight*>(m_gr->allocateFrameTransientMemory(
				sizeof(ShaderPointLight) * visiblePointLightsCount,
				BufferUsage::UNIFORM,
				pointLightsToken));

		ctx.m_pointLights =
			WeakArray<ShaderPointLight>(data, visiblePointLightsCount);

		ctx.m_vPointLights = WeakArray<VisibleNode>(
			vi.getBegin(VisibilityGroupType::LIGHTS_POINT),
			visiblePointLightsCount);
	}
	else
	{
		pointLightsToken.markUnused();
	}

	if(visibleSpotLightsCount)
	{
		ShaderSpotLight* data =
			static_cast<ShaderSpotLight*>(m_gr->allocateFrameTransientMemory(
				sizeof(ShaderSpotLight) * visibleSpotLightsCount,
				BufferUsage::UNIFORM,
				spotLightsToken));

		ctx.m_spotLights =
			WeakArray<ShaderSpotLight>(data, visibleSpotLightsCount);

		ctx.m_vSpotLights = WeakArray<VisibleNode>(
			vi.getBegin(VisibilityGroupType::LIGHTS_SPOT),
			visibleSpotLightsCount);
	}
	else
	{
		spotLightsToken.markUnused();
	}

	if(probesToken)
	{
		if(visibleProbeCount)
		{
			ShaderProbe* data =
				static_cast<ShaderProbe*>(m_gr->allocateFrameTransientMemory(
					sizeof(ShaderProbe) * visibleProbeCount,
					BufferUsage::UNIFORM,
					*probesToken));
	
			ctx.m_probes = WeakArray<ShaderProbe>(data, visibleProbeCount);
	
			ctx.m_vProbes = WeakArray<VisibleNode>(
				vi.getBegin(VisibilityGroupType::REFLECTION_PROBES),
				visibleProbeCount);
		}
		else
		{
			probesToken->markUnused();
		}
	}

	ctx.m_bin = this;

	// Get mem for clusters
	ShaderCluster* data =
		static_cast<ShaderCluster*>(m_gr->allocateFrameTransientMemory(
			sizeof(ShaderCluster) * m_clusterCount,
			BufferUsage::STORAGE,
			clustersToken));

	ctx.m_clusters = WeakArray<ShaderCluster>(data, m_clusterCount);

	// Allocate light IDs
	U32* data2 = static_cast<U32*>(
		m_gr->allocateFrameTransientMemory(maxLightIndices * sizeof(U32),
			BufferUsage::STORAGE,
			lightIndicesToken));

	ctx.m_lightIds = WeakArray<U32>(data2, maxLightIndices);

	// Fire the async job
	for(U i = 0; i < m_threadPool->getThreadsCount(); i++)
	{
		tasks[i].m_ctx = &ctx;

		m_threadPool->assignNewTask(i, &tasks[i]);
	}

	// Sync
	ANKI_CHECK(m_threadPool->waitForAllThreadsToFinish());

	ANKI_TRACE_STOP_EVENT(RENDER_LIGHT_BINNING);
	return ErrorCode::NONE;
}

//==============================================================================
void LightBin::binLights(
	U32 threadId, PtrSize threadsCount, LightBinContext& ctx)
{
	ANKI_TRACE_START_EVENT(RENDER_LIGHT_BINNING);
	const FrustumComponent& camfrc = *ctx.m_frc;
	const MoveComponent& cammove =
		camfrc.getSceneNode().getComponent<MoveComponent>();
	U clusterCount = m_clusterCount;
	PtrSize start, end;

	//
	// Initialize the temp clusters
	//
	ThreadPoolTask::choseStartEnd(
		threadId, threadsCount, clusterCount, start, end);

	for(U i = start; i < end; ++i)
	{
		ctx.m_tempClusters[i].reset();
	}

	ANKI_TRACE_STOP_EVENT(RENDER_LIGHT_BINNING);
	m_barrier.wait();
	ANKI_TRACE_START_EVENT(RENDER_LIGHT_BINNING);

	//
	// Iterate lights and probes and bin them
	//
	ClustererTestResult testResult;
	m_clusterer.initTestResults(ctx.m_alloc, testResult);
	U lightCount = ctx.m_vPointLights.getSize() + ctx.m_vSpotLights.getSize();
	U totalCount = lightCount + ctx.m_vProbes.getSize();

	const U TO_BIN_COUNT = 1;
	while((start = ctx.m_count2.fetchAdd(TO_BIN_COUNT)) < totalCount)
	{
		end = min<U>(start + TO_BIN_COUNT, totalCount);

		for(U j = start; j < end; ++j)
		{
			if(j >= lightCount)
			{
				U i = j - lightCount;
				SceneNode& snode = *ctx.m_vProbes[i].m_node;
				writeAndBinProbe(camfrc, snode, ctx, testResult);
			}
			else if(j >= ctx.m_vPointLights.getSize())
			{
				U i = j - ctx.m_vPointLights.getSize();

				SceneNode& snode = *ctx.m_vSpotLights[i].m_node;
				MoveComponent& move = snode.getComponent<MoveComponent>();
				LightComponent& light = snode.getComponent<LightComponent>();
				SpatialComponent& sp = snode.getComponent<SpatialComponent>();
				const FrustumComponent* frc =
					snode.tryGetComponent<FrustumComponent>();

				I pos = writeSpotLight(light, move, frc, cammove, camfrc, ctx);
				binLight(sp, pos, 1, ctx, testResult);
			}
			else
			{
				U i = j;

				SceneNode& snode = *ctx.m_vPointLights[i].m_node;
				MoveComponent& move = snode.getComponent<MoveComponent>();
				LightComponent& light = snode.getComponent<LightComponent>();
				SpatialComponent& sp = snode.getComponent<SpatialComponent>();

				I pos = writePointLight(light, move, camfrc, ctx);
				binLight(sp, pos, 0, ctx, testResult);
			}
		}
	}

	//
	// Last thing, update the real clusters
	//
	ANKI_TRACE_STOP_EVENT(RENDER_LIGHT_BINNING);
	m_barrier.wait();
	ANKI_TRACE_START_EVENT(RENDER_LIGHT_BINNING);

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
			const U count = countP + countS + countProbe;

			auto& c = ctx.m_clusters[i];
			c.m_combo = 0;

			// Early exit
			if(ANKI_UNLIKELY(count == 0))
			{
				continue;
			}

			// Check if the previous cluster contains the same lights as this
			// one and if yes then merge them. This will avoid allocating new
			// IDs (and thrashing GPU caches).
			cluster.sortLightIds();
			if(i != start)
			{
				const auto& clusterB = ctx.m_tempClusters[i - 1];

				if(cluster == clusterB)
				{
					c.m_combo = ctx.m_clusters[i - 1].m_combo;
					continue;
				}
			}

			U offset = ctx.m_lightIdsCount.fetchAdd(count);

			if(offset + count <= ctx.m_maxLightIndices)
			{
				ANKI_ASSERT(offset <= 0xFFFF);
				c.m_combo = offset << 16;

				if(countP > 0)
				{
					ANKI_ASSERT(countP <= 0xF);
					c.m_combo |= countP << 4;

					for(U i = 0; i < countP; ++i)
					{
						ctx.m_lightIds[offset++] =
							cluster.m_pointIds[i].getIndex();
					}
				}

				if(countS > 0)
				{
					ANKI_ASSERT(countS <= 0xF);
					c.m_combo |= countS;

					for(U i = 0; i < countS; ++i)
					{
						ctx.m_lightIds[offset++] =
							cluster.m_spotIds[i].getIndex();
					}
				}

				if(countProbe > 0)
				{
					ANKI_ASSERT(countProbe <= 0xF);
					c.m_combo |= countProbe << 8;

					for(U i = 0; i < countProbe; ++i)
					{
						ctx.m_lightIds[offset++] =
							cluster.m_probeIds[i].getIndex();
					}
				}
			}
			else
			{
				ANKI_LOGW("Light IDs buffer too small");
			}
		} // end for
	} // end while

	ANKI_TRACE_STOP_EVENT(RENDER_LIGHT_BINNING);
}

//==============================================================================
I LightBin::writePointLight(const LightComponent& lightc,
	const MoveComponent& lightMove,
	const FrustumComponent& camFrc,
	LightBinContext& ctx)
{
	// Get GPU light
	I i = ctx.m_pointLightsCount.fetchAdd(1);

	ShaderPointLight& slight = ctx.m_pointLights[i];

	Vec4 pos = camFrc.getViewMatrix()
		* lightMove.getWorldTransform().getOrigin().xyz1();

	slight.m_posRadius =
		Vec4(pos.xyz(), 1.0 / (lightc.getRadius() * lightc.getRadius()));
	slight.m_diffuseColorShadowmapId = lightc.getDiffuseColor();

	if(!lightc.getShadowEnabled() || !ctx.m_shadowsEnabled)
	{
		slight.m_diffuseColorShadowmapId.w() = INVALID_TEXTURE_INDEX;
	}
	else
	{
		slight.m_diffuseColorShadowmapId.w() = lightc.getShadowMapIndex();
	}

	slight.m_specularColorTexId = lightc.getSpecularColor();

	return i;
}

//==============================================================================
I LightBin::writeSpotLight(const LightComponent& lightc,
	const MoveComponent& lightMove,
	const FrustumComponent* lightFrc,
	const MoveComponent& camMove,
	const FrustumComponent& camFrc,
	LightBinContext& ctx)
{
	I i = ctx.m_spotLightsCount.fetchAdd(1);

	ShaderSpotLight& light = ctx.m_spotLights[i];
	F32 shadowmapIndex = INVALID_TEXTURE_INDEX;

	if(lightc.getShadowEnabled() && ctx.m_shadowsEnabled)
	{
		// Write matrix
		static const Mat4 biasMat4(0.5,
			0.0,
			0.0,
			0.5,
			0.0,
			0.5,
			0.0,
			0.5,
			0.0,
			0.0,
			0.5,
			0.5,
			0.0,
			0.0,
			0.0,
			1.0);
		// bias * proj_l * view_l * world_c
		light.m_texProjectionMat = biasMat4
			* lightFrc->getViewProjectionMatrix()
			* Mat4(camMove.getWorldTransform());

		shadowmapIndex = (F32)lightc.getShadowMapIndex();
	}

	// Pos & dist
	Vec4 pos = camFrc.getViewMatrix()
		* lightMove.getWorldTransform().getOrigin().xyz1();
	light.m_posRadius =
		Vec4(pos.xyz(), 1.0 / (lightc.getDistance() * lightc.getDistance()));

	// Diff color and shadowmap ID now
	light.m_diffuseColorShadowmapId =
		Vec4(lightc.getDiffuseColor().xyz(), shadowmapIndex);

	// Spec color
	light.m_specularColorTexId = lightc.getSpecularColor();

	// Light dir
	Vec3 lightDir = -lightMove.getWorldTransform().getRotation().getZAxis();
	lightDir = camFrc.getViewMatrix().getRotationPart() * lightDir;
	light.m_lightDir = Vec4(lightDir, 0.0);

	// Angles
	light.m_outerCosInnerCos =
		Vec4(lightc.getOuterAngleCos(), lightc.getInnerAngleCos(), 1.0, 1.0);

	return i;
}

//==============================================================================
void LightBin::binLight(SpatialComponent& sp,
	U pos,
	U lightType,
	LightBinContext& ctx,
	ClustererTestResult& testResult)
{
	m_clusterer.bin(sp.getSpatialCollisionShape(), sp.getAabb(), testResult);

	// Bin to the correct tiles
	auto it = testResult.getClustersBegin();
	auto end = testResult.getClustersEnd();
	for(; it != end; ++it)
	{
		U x = (*it)[0];
		U y = (*it)[1];
		U z = (*it)[2];

		U i = m_clusterer.getClusterCountX()
				* (z * m_clusterer.getClusterCountY() + y)
			+ x;

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

//==============================================================================
void LightBin::writeAndBinProbe(const FrustumComponent& camFrc,
	const SceneNode& node,
	LightBinContext& ctx,
	ClustererTestResult& testResult)
{
	const ReflectionProbeComponent& reflc =
		node.getComponent<ReflectionProbeComponent>();
	const SpatialComponent& sp = node.getComponent<SpatialComponent>();

	// Write it
	ShaderProbe probe;
	probe.m_pos = (camFrc.getViewMatrix() * reflc.getPosition().xyz1()).xyz();
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
		U x = (*it)[0];
		U y = (*it)[1];
		U z = (*it)[2];

		U i = m_clusterer.getClusterCountX()
				* (z * m_clusterer.getClusterCountY() + y)
			+ x;

		auto& cluster = ctx.m_tempClusters[i];

		i = cluster.m_probeCount.fetchAdd(1) % MAX_PROBES_PER_CLUSTER;
		cluster.m_probeIds[i].setIndex(idx);
		cluster.m_probeIds[i].setProbeRadius(reflc.getRadius());
	}
}

} // end namespace anki
