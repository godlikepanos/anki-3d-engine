// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Is.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Ms.h>
#include <anki/renderer/Sm.h>
#include <anki/renderer/Pps.h>
#include <anki/renderer/Ir.h>
#include <anki/scene/Camera.h>
#include <anki/scene/Light.h>
#include <anki/scene/ReflectionProbeComponent.h>
#include <anki/scene/Visibility.h>
#include <anki/core/Trace.h>
#include <anki/util/Logger.h>
#include <anki/misc/ConfigSet.h>

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
};

struct ShaderCommonUniforms
{
	Vec4 m_projectionParams;
	Vec4 m_sceneAmbientColor;
	Vec4 m_rendererSizeTimePad1;
	Vec4 m_nearFarClustererMagicPad1;
	Mat4 m_viewMat;
	Mat3x4 m_invViewRotation;
	UVec4 m_tileCount;
};

static const U MAX_TYPED_LIGHTS_PER_CLUSTER = 16;
static const U MAX_PROBES_PER_CLUSTER = 8;
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
		if(a > maxCount)
		{
			ANKI_LOGW("Increase cluster limit: %s", &what[0]);
		}
	}
};

/// Common data for all tasks.
class TaskCommonData
{
public:
	TaskCommonData(StackAllocator<U8> alloc)
		: m_tempClusters(alloc)
	{
	}

	// To fill the light buffers
	SArray<ShaderPointLight> m_pointLights;
	SArray<ShaderSpotLight> m_spotLights;
	SArray<ShaderProbe> m_probes;

	SArray<U32> m_lightIds;
	SArray<ShaderCluster> m_clusters;

	Atomic<U32> m_pointLightsCount = {0};
	Atomic<U32> m_spotLightsCount = {0};
	Atomic<U32> m_probeCount = {0};

	// To fill the tile buffers
	DArrayAuto<ClusterData> m_tempClusters;

	// To fill the light index buffer
	Atomic<U32> m_lightIdsCount = {0};

	// Misc
	SArray<VisibleNode> m_vPointLights;
	SArray<VisibleNode> m_vSpotLights;
	SArray<VisibleNode> m_vProbes;

	Atomic<U32> m_count = {0};
	Atomic<U32> m_count2 = {0};

	Is* m_is = nullptr;
};

/// Write the lights to the GPU buffers.
class WriteLightsTask : public ThreadPool::Task
{
public:
	TaskCommonData* m_data = nullptr;

	Error operator()(U32 threadId, PtrSize threadsCount)
	{
		m_data->m_is->binLights(threadId, threadsCount, *m_data);
		return ErrorCode::NONE;
	}
};

//==============================================================================
// Is                                                                          =
//==============================================================================

const PixelFormat Is::RT_PIXEL_FORMAT(
	ComponentFormat::R11G11B10, TransformFormat::FLOAT);

//==============================================================================
Is::Is(Renderer* r)
	: RenderingPass(r)
{
}

//==============================================================================
Is::~Is()
{
	if(m_barrier)
	{
		getAllocator().deleteInstance(m_barrier.get());
	}
}

//==============================================================================
Error Is::init(const ConfigSet& config)
{
	Error err = initInternal(config);

	if(err)
	{
		ANKI_LOGE("Failed to init IS");
	}

	return err;
}

//==============================================================================
Error Is::initInternal(const ConfigSet& config)
{
	m_groundLightEnabled = config.getNumber("is.groundLightEnabled");

	m_maxLightIds = config.getNumber("is.maxLightsPerCluster");

	if(m_maxLightIds == 0)
	{
		ANKI_LOGE("Incorrect number of max light indices");
		return ErrorCode::USER_DATA;
	}

	m_maxLightIds *= m_r->getClusterCount();

	//
	// Load the programs
	//
	StringAuto pps(getAllocator());

	pps.sprintf("\n#define TILE_COUNT_X %u\n"
				"#define TILE_COUNT_Y %u\n"
				"#define CLUSTER_COUNT %u\n"
				"#define RENDERER_WIDTH %u\n"
				"#define RENDERER_HEIGHT %u\n"
				"#define MAX_LIGHT_INDICES %u\n"
				"#define GROUND_LIGHT %u\n"
				"#define POISSON %u\n"
				"#define INDIRECT_ENABLED %u\n"
				"#define IR_MIPMAP_COUNT %u\n",
		m_r->getTileCountXY().x(),
		m_r->getTileCountXY().y(),
		m_r->getClusterCount(),
		m_r->getWidth(),
		m_r->getHeight(),
		m_maxLightIds,
		m_groundLightEnabled,
		m_r->getSmEnabled() ? m_r->getSm().getPoissonEnabled() : 0,
		m_r->getIrEnabled(),
		(m_r->getIrEnabled()) ? m_r->getIr().getCubemapArrayMipmapCount() : 0);

	// point light
	ANKI_CHECK(getResourceManager().loadResourceToCache(
		m_lightVert, "shaders/Is.vert.glsl", pps.toCString(), "r_"));

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		m_lightFrag, "shaders/Is.frag.glsl", pps.toCString(), "r_"));

	PipelineInitInfo init;

	init.m_inputAssembler.m_topology = PrimitiveTopology::TRIANGLE_STRIP;
	init.m_depthStencil.m_depthWriteEnabled = false;
	init.m_depthStencil.m_depthCompareFunction = CompareOperation::ALWAYS;
	init.m_color.m_attachmentCount = 1;
	init.m_color.m_attachments[0].m_format = RT_PIXEL_FORMAT;
	init.m_shaders[U(ShaderType::VERTEX)] = m_lightVert->getGrShader();
	init.m_shaders[U(ShaderType::FRAGMENT)] = m_lightFrag->getGrShader();
	m_lightPpline = getGrManager().newInstance<Pipeline>(init);

	//
	// Create framebuffer
	//
	m_r->createRenderTarget(m_r->getWidth(),
		m_r->getHeight(),
		RT_PIXEL_FORMAT,
		1,
		SamplingFilter::LINEAR,
		IS_MIPMAP_COUNT,
		m_rt);

	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation =
		AttachmentLoadOperation::DONT_CARE;
	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	//
	// Create resource group
	//
	{
		ResourceGroupInitInfo init;
		init.m_textures[0].m_texture = m_r->getMs().getRt0();
		init.m_textures[1].m_texture = m_r->getMs().getRt1();
		init.m_textures[2].m_texture = m_r->getMs().getRt2();
		init.m_textures[3].m_texture = m_r->getMs().getDepthRt();

		if(m_r->getSmEnabled())
		{
			init.m_textures[4].m_texture = m_r->getSm().getSpotTextureArray();
			init.m_textures[5].m_texture = m_r->getSm().getOmniTextureArray();
		}

		if(m_r->getIrEnabled())
		{
			init.m_textures[6].m_texture = m_r->getIr().getReflectionTexture();
			init.m_textures[7].m_texture = m_r->getIr().getIrradianceTexture();

			init.m_textures[8].m_texture = m_r->getIr().getIntegrationLut();
			init.m_textures[8].m_sampler =
				m_r->getIr().getIntegrationLutSampler();
		}

		init.m_uniformBuffers[0].m_dynamic = true;

		init.m_storageBuffers[0].m_dynamic = true;
		init.m_storageBuffers[1].m_dynamic = true;
		init.m_storageBuffers[2].m_dynamic = true;
		init.m_storageBuffers[3].m_dynamic = true;
		init.m_storageBuffers[4].m_dynamic = true;

		m_rcGroup = getGrManager().newInstance<ResourceGroup>(init);
	}

	//
	// Misc
	//
	ThreadPool& threadPool = m_r->getThreadPool();
	m_barrier =
		getAllocator().newInstance<Barrier>(threadPool.getThreadsCount());

	getGrManager().finish();
	return ErrorCode::NONE;
}

//==============================================================================
Error Is::lightPass(RenderingContext& ctx)
{
	ANKI_TRACE_START_EVENT(RENDER_IS);
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;
	ThreadPool& threadPool = m_r->getThreadPool();
	m_frc = ctx.m_frustumComponent;
	VisibilityTestResults& vi = m_frc->getVisibilityTestResults();

	U clusterCount = m_r->getClusterCount();

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
	TaskCommonData taskData(getFrameAllocator());
	taskData.m_tempClusters.create(m_r->getClusterCount());

	if(visiblePointLightsCount)
	{
		ShaderPointLight* data = static_cast<ShaderPointLight*>(
			getGrManager().allocateFrameHostVisibleMemory(
				sizeof(ShaderPointLight) * visiblePointLightsCount,
				BufferUsage::STORAGE,
				m_pLightsToken));

		taskData.m_pointLights =
			SArray<ShaderPointLight>(data, visiblePointLightsCount);

		taskData.m_vPointLights =
			SArray<VisibleNode>(vi.getBegin(VisibilityGroupType::LIGHTS_POINT),
				visiblePointLightsCount);
	}
	else
	{
		m_pLightsToken.markUnused();
	}

	if(visibleSpotLightsCount)
	{
		ShaderSpotLight* data = static_cast<ShaderSpotLight*>(
			getGrManager().allocateFrameHostVisibleMemory(
				sizeof(ShaderSpotLight) * visibleSpotLightsCount,
				BufferUsage::STORAGE,
				m_sLightsToken));

		taskData.m_spotLights =
			SArray<ShaderSpotLight>(data, visibleSpotLightsCount);

		taskData.m_vSpotLights =
			SArray<VisibleNode>(vi.getBegin(VisibilityGroupType::LIGHTS_SPOT),
				visibleSpotLightsCount);
	}
	else
	{
		m_sLightsToken.markUnused();
	}

	if(m_r->getIrEnabled() && visibleProbeCount)
	{
		ShaderProbe* data = static_cast<ShaderProbe*>(
			getGrManager().allocateFrameHostVisibleMemory(
				sizeof(ShaderProbe) * visibleProbeCount,
				BufferUsage::STORAGE,
				m_probesToken));

		taskData.m_probes = SArray<ShaderProbe>(data, visibleProbeCount);

		taskData.m_vProbes = SArray<VisibleNode>(
			vi.getBegin(VisibilityGroupType::REFLECTION_PROBES),
			visibleProbeCount);
	}
	else
	{
		m_probesToken.markUnused();
	}

	taskData.m_is = this;

	// Get mem for clusters
	ShaderCluster* data = static_cast<ShaderCluster*>(
		getGrManager().allocateFrameHostVisibleMemory(
			sizeof(ShaderCluster) * clusterCount,
			BufferUsage::STORAGE,
			m_clustersToken));

	taskData.m_clusters = SArray<ShaderCluster>(data, clusterCount);

	// Allocate light IDs
	U32* data2 =
		static_cast<U32*>(getGrManager().allocateFrameHostVisibleMemory(
			m_maxLightIds * sizeof(U32),
			BufferUsage::STORAGE,
			m_lightIdsToken));

	taskData.m_lightIds = SArray<U32>(data2, m_maxLightIds);

	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		tasks[i].m_data = &taskData;

		threadPool.assignNewTask(i, &tasks[i]);
	}

	// Update uniforms
	updateCommonBlock(*m_frc);

	// In the meantime set the state
	setState(cmdb);

	// Sync
	ANKI_CHECK(threadPool.waitForAllThreadsToFinish());

	//
	// Draw
	//
	cmdb->drawArrays(4, m_r->getTileCount());
	cmdb->endRenderPass();

	ANKI_TRACE_STOP_EVENT(RENDER_IS);
	return ErrorCode::NONE;
}

//==============================================================================
void Is::binLights(U32 threadId, PtrSize threadsCount, TaskCommonData& task)
{
	ANKI_TRACE_START_EVENT(RENDER_IS);
	const FrustumComponent& camfrc = *m_frc;
	const MoveComponent& cammove =
		m_frc->getSceneNode().getComponent<MoveComponent>();
	U clusterCount = m_r->getClusterCount();
	PtrSize start, end;

	//
	// Initialize the temp clusters
	//
	ThreadPool::Task::choseStartEnd(
		threadId, threadsCount, clusterCount, start, end);

	for(U i = start; i < end; ++i)
	{
		task.m_tempClusters[i].reset();
	}

	ANKI_TRACE_STOP_EVENT(RENDER_IS);
	m_barrier->wait();
	ANKI_TRACE_START_EVENT(RENDER_IS);

	//
	// Iterate lights and probes and bin them
	//
	ClustererTestResult testResult;
	m_r->getClusterer().initTestResults(getFrameAllocator(), testResult);
	U lightCount = task.m_vPointLights.getSize() + task.m_vSpotLights.getSize();
	U totalCount = lightCount + task.m_vProbes.getSize();

	const U TO_BIN_COUNT = 1;
	while((start = task.m_count2.fetchAdd(TO_BIN_COUNT)) < totalCount)
	{
		end = min<U>(start + TO_BIN_COUNT, totalCount);

		for(U j = start; j < end; ++j)
		{
			if(j >= lightCount)
			{
				U i = j - lightCount;
				SceneNode& snode = *task.m_vProbes[i].m_node;
				writeAndBinProbe(camfrc, snode, task, testResult);
			}
			else if(j >= task.m_vPointLights.getSize())
			{
				U i = j - task.m_vPointLights.getSize();

				SceneNode& snode = *task.m_vSpotLights[i].m_node;
				MoveComponent& move = snode.getComponent<MoveComponent>();
				LightComponent& light = snode.getComponent<LightComponent>();
				SpatialComponent& sp = snode.getComponent<SpatialComponent>();
				const FrustumComponent* frc =
					snode.tryGetComponent<FrustumComponent>();

				I pos = writeSpotLight(light, move, frc, cammove, camfrc, task);
				binLight(sp, pos, 1, task, testResult);
			}
			else
			{
				U i = j;

				SceneNode& snode = *task.m_vPointLights[i].m_node;
				MoveComponent& move = snode.getComponent<MoveComponent>();
				LightComponent& light = snode.getComponent<LightComponent>();
				SpatialComponent& sp = snode.getComponent<SpatialComponent>();

				I pos = writePointLight(light, move, camfrc, task);
				binLight(sp, pos, 0, task, testResult);
			}
		}
	}

	//
	// Last thing, update the real clusters
	//
	ANKI_TRACE_STOP_EVENT(RENDER_IS);
	m_barrier->wait();
	ANKI_TRACE_START_EVENT(RENDER_IS);

	// Run per cluster
	const U CLUSTER_GROUP = 16;
	while((start = task.m_count.fetchAdd(CLUSTER_GROUP)) < clusterCount)
	{
		end = min<U>(start + CLUSTER_GROUP, clusterCount);

		for(U i = start; i < end; ++i)
		{
			auto& cluster = task.m_tempClusters[i];
			cluster.normalizeCounts();

			const U countP = cluster.m_pointCount.get();
			const U countS = cluster.m_spotCount.get();
			const U countProbe = cluster.m_probeCount.get();
			const U count = countP + countS + countProbe;

			auto& c = task.m_clusters[i];
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
				const auto& clusterB = task.m_tempClusters[i - 1];

				if(cluster == clusterB)
				{
					c.m_combo = task.m_clusters[i - 1].m_combo;
					continue;
				}
			}

			U offset = task.m_lightIdsCount.fetchAdd(count);

			if(offset + count <= m_maxLightIds)
			{
				ANKI_ASSERT(offset <= 0xFFFF);
				c.m_combo = offset << 16;

				if(countP > 0)
				{
					ANKI_ASSERT(countP <= 0xF);
					c.m_combo |= countP << 4;

					for(U i = 0; i < countP; ++i)
					{
						task.m_lightIds[offset++] =
							cluster.m_pointIds[i].getIndex();
					}
				}

				if(countS > 0)
				{
					ANKI_ASSERT(countS <= 0xF);
					c.m_combo |= countS;

					for(U i = 0; i < countS; ++i)
					{
						task.m_lightIds[offset++] =
							cluster.m_spotIds[i].getIndex();
					}
				}

				if(countProbe > 0)
				{
					ANKI_ASSERT(countProbe <= 0xF);
					c.m_combo |= countProbe << 8;

					for(U i = 0; i < countProbe; ++i)
					{
						task.m_lightIds[offset++] =
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

	ANKI_TRACE_STOP_EVENT(RENDER_IS);
}

//==============================================================================
I Is::writePointLight(const LightComponent& lightc,
	const MoveComponent& lightMove,
	const FrustumComponent& camFrc,
	TaskCommonData& task)
{
	// Get GPU light
	I i = task.m_pointLightsCount.fetchAdd(1);

	ShaderPointLight& slight = task.m_pointLights[i];

	Vec4 pos = camFrc.getViewMatrix()
		* lightMove.getWorldTransform().getOrigin().xyz1();

	slight.m_posRadius =
		Vec4(pos.xyz(), 1.0 / (lightc.getRadius() * lightc.getRadius()));
	slight.m_diffuseColorShadowmapId = lightc.getDiffuseColor();

	if(!lightc.getShadowEnabled() || !m_r->getSmEnabled())
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
I Is::writeSpotLight(const LightComponent& lightc,
	const MoveComponent& lightMove,
	const FrustumComponent* lightFrc,
	const MoveComponent& camMove,
	const FrustumComponent& camFrc,
	TaskCommonData& task)
{
	I i = task.m_spotLightsCount.fetchAdd(1);

	ShaderSpotLight& light = task.m_spotLights[i];
	F32 shadowmapIndex = INVALID_TEXTURE_INDEX;

	if(lightc.getShadowEnabled() && m_r->getSmEnabled())
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
void Is::binLight(SpatialComponent& sp,
	U pos,
	U lightType,
	TaskCommonData& task,
	ClustererTestResult& testResult)
{
	m_r->getClusterer().bin(
		sp.getSpatialCollisionShape(), sp.getAabb(), testResult);

	// Bin to the correct tiles
	auto it = testResult.getClustersBegin();
	auto end = testResult.getClustersEnd();
	for(; it != end; ++it)
	{
		U x = (*it)[0];
		U y = (*it)[1];
		U z = (*it)[2];

		U i =
			m_r->getTileCountXY().x() * (z * m_r->getTileCountXY().y() + y) + x;

		auto& cluster = task.m_tempClusters[i];

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
void Is::writeAndBinProbe(const FrustumComponent& camFrc,
	const SceneNode& node,
	TaskCommonData& task,
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

	U idx = task.m_probeCount.fetchAdd(1);
	task.m_probes[idx] = probe;

	// Bin it
	m_r->getClusterer().bin(
		sp.getSpatialCollisionShape(), sp.getAabb(), testResult);

	auto it = testResult.getClustersBegin();
	auto end = testResult.getClustersEnd();
	for(; it != end; ++it)
	{
		U x = (*it)[0];
		U y = (*it)[1];
		U z = (*it)[2];

		U i = m_r->getClusterer().getClusterCountX()
				* (z * m_r->getClusterer().getClusterCountY() + y)
			+ x;

		auto& cluster = task.m_tempClusters[i];

		i = cluster.m_probeCount.fetchAdd(1) % MAX_PROBES_PER_CLUSTER;
		cluster.m_probeIds[i].setIndex(idx);
		cluster.m_probeIds[i].setProbeRadius(reflc.getRadius());
	}
}

//==============================================================================
void Is::setState(CommandBufferPtr& cmdb)
{
	cmdb->beginRenderPass(m_fb);
	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	cmdb->bindPipeline(m_lightPpline);

	DynamicBufferInfo dyn;
	dyn.m_uniformBuffers[0] = m_commonVarsToken;

	dyn.m_storageBuffers[0] = m_pLightsToken;
	dyn.m_storageBuffers[1] = m_sLightsToken;
	dyn.m_storageBuffers[2] = m_clustersToken;
	dyn.m_storageBuffers[3] = m_lightIdsToken;
	dyn.m_storageBuffers[4] = m_probesToken;

	cmdb->bindResourceGroup(m_rcGroup, 0, &dyn);
}

//==============================================================================
Error Is::run(RenderingContext& ctx)
{
	// Do the light pass including the shadow passes
	return lightPass(ctx);
}

//==============================================================================
void Is::updateCommonBlock(const FrustumComponent& fr)
{
	ShaderCommonUniforms* blk = static_cast<ShaderCommonUniforms*>(
		getGrManager().allocateFrameHostVisibleMemory(
			sizeof(ShaderCommonUniforms),
			BufferUsage::UNIFORM,
			m_commonVarsToken));

	// Start writing
	blk->m_projectionParams = fr.getProjectionParameters();
	blk->m_sceneAmbientColor = m_ambientColor;
	blk->m_viewMat = fr.getViewMatrix().getTransposed();
	blk->m_nearFarClustererMagicPad1 = Vec4(fr.getFrustum().getNear(),
		fr.getFrustum().getFar(),
		m_r->getClusterer().getShaderMagicValue(),
		0.0);

	blk->m_invViewRotation =
		Mat3x4(fr.getViewMatrix().getInverse().getRotationPart());

	blk->m_rendererSizeTimePad1 = Vec4(
		m_r->getWidth(), m_r->getHeight(), HighRezTimer::getCurrentTime(), 0.0);

	blk->m_tileCount = UVec4(m_r->getTileCountXY(), m_r->getTileCount(), 0);
}

} // end namespace anki
