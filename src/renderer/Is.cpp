// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Is.h"
#include "anki/renderer/Renderer.h"
#include "anki/renderer/Ms.h"
#include "anki/renderer/Pps.h"
#include "anki/renderer/Tiler.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Light.h"
#include "anki/scene/Visibility.h"
#include "anki/core/Counters.h"
#include "anki/util/Logger.h"
#include "anki/misc/ConfigSet.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

// Shader structs and block representations. All positions and directions in
// viewspace
// For documentation see the shaders

struct ShaderCluster
{
	/// If m_combo = 0xFFFFAABB then FFFF is the light index offset, AA the
	/// number of point lights and BB the number of spot lights
	U32 m_combo;
};

struct ShaderLight
{
	Vec4 m_posRadius;
	Vec4 m_diffuseColorShadowmapId;
	Vec4 m_specularColorTexId;
};

struct ShaderPointLight: ShaderLight
{};

struct ShaderSpotLight: ShaderLight
{
	Vec4 m_lightDir;
	Vec4 m_outerCosInnerCos;
	Mat4 m_texProjectionMat; ///< Texture projection matrix
};

struct ShaderCommonUniforms
{
	Vec4 m_projectionParams;
	Vec4 m_sceneAmbientColor;
	Vec4 m_groundLightDir;
	Vec4 m_nearFarClustererDivisor;
	Mat4 m_viewMat;
	UVec4 m_tileCount;
};

using Lid = U32; ///< Light ID
static const U MAX_TYPED_LIGHTS_PER_CLUSTER = 16;
static const F32 INVALID_TEXTURE_INDEX = 128.0;

class ClusterData
{
public:
	Atomic<U32> m_pointCount;
	Atomic<U32> m_spotCount;

	Array<Lid, MAX_TYPED_LIGHTS_PER_CLUSTER> m_pointIds;
	Array<Lid, MAX_TYPED_LIGHTS_PER_CLUSTER> m_spotIds;

	void sortLightIds()
	{
		const U pointCount = m_pointCount.load();
		const U spotCount = m_spotCount.load();

		if(pointCount > 1)
		{
			std::sort(&m_pointIds[0], &m_pointIds[0] + pointCount);
		}

		if(spotCount > 1)
		{
			std::sort(&m_spotIds[0], &m_spotIds[0] + spotCount);
		}
	}

	Bool operator==(const ClusterData& b) const
	{
		const U pointCount = m_pointCount.load();
		const U spotCount = m_spotCount.load();
		const U pointCount2 = b.m_pointCount.load();
		const U spotCount2 = b.m_spotCount.load();

		if(pointCount != pointCount2 || spotCount != spotCount2)
		{
			return false;
		}

		if(pointCount > 0)
		{
			if(memcmp(&m_pointIds[0], &b.m_pointIds[0],
				sizeof(Lid) * pointCount) != 0)
			{
				return false;
			}
		}

		if(spotCount > 0)
		{
			if(memcmp(&m_spotIds[0], &b.m_spotIds[0],
				sizeof(Lid) * spotCount) != 0)
			{
				return false;
			}
		}

		return true;
	}
};

/// Common data for all tasks.
class TaskCommonData
{
public:
	TaskCommonData(StackAllocator<U8> alloc)
		: m_tempClusters(alloc)
	{}

	// To fill the light buffers
	SArray<ShaderPointLight> m_pointLights;
	SArray<ShaderSpotLight> m_spotLights;

	SArray<Lid> m_lightIds;
	SArray<ShaderCluster> m_clusters;

	Atomic<U32> m_pointLightsCount = {0};
	Atomic<U32> m_spotLightsCount = {0};

	// To fill the tile buffers
	DArrayAuto<ClusterData> m_tempClusters;

	// To fill the light index buffer
	Atomic<U32> m_lightIdsCount = {0};

	// Misc
	VisibilityTestResults::Container::ConstIterator m_lightsBegin = nullptr;
	VisibilityTestResults::Container::ConstIterator m_lightsEnd = nullptr;

	Is* m_is = nullptr;
};

/// Write the lights to the GPU buffers.
class WriteLightsTask: public ThreadPool::Task
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
	, m_sm(r)
{}

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
	m_maxPointLights = config.getNumber("is.maxPointLights");
	m_maxSpotLights = config.getNumber("is.maxSpotLights");
	m_maxSpotTexLights = config.getNumber("is.maxSpotTexLights");

	if(m_maxPointLights < 1 || m_maxSpotLights < 1 || m_maxSpotTexLights < 1)
	{
		ANKI_LOGE("Incorrect number of max lights");
		return ErrorCode::USER_DATA;
	}

	m_maxLightIds = config.getNumber("is.maxLightsPerCluster");

	if(m_maxLightIds == 0)
	{
		ANKI_LOGE("Incorrect number of max light indices");
		return ErrorCode::USER_DATA;
	}

	m_maxLightIds *= m_r->getClusterCount();

	//
	// Init the passes
	//
	ANKI_CHECK(m_sm.init(config));

	//
	// Load the programs
	//
	StringAuto pps(getAllocator());

	pps.sprintf(
		"\n#define TILES_COUNT_X %u\n"
		"#define TILES_COUNT_Y %u\n"
		"#define CLUSTER_COUNT %u\n"
		"#define RENDERER_WIDTH %u\n"
		"#define RENDERER_HEIGHT %u\n"
		"#define MAX_POINT_LIGHTS %u\n"
		"#define MAX_SPOT_LIGHTS %u\n"
		"#define MAX_LIGHT_INDICES %u\n"
		"#define GROUND_LIGHT %u\n"
		"#define POISSON %u\n",
		m_r->getTileCountXY().x(),
		m_r->getTileCountXY().y(),
		m_r->getClusterCount(),
		m_r->getWidth(),
		m_r->getHeight(),
		m_maxPointLights,
		m_maxSpotLights,
		m_maxLightIds,
		m_groundLightEnabled,
		m_sm.getPoissonEnabled());

	// point light
	ANKI_CHECK(getResourceManager().loadResourceToCache(m_lightVert,
		"shaders/IsLp.vert.glsl", pps.toCString(), "r_"));

	ANKI_CHECK(getResourceManager().loadResourceToCache(m_lightFrag,
		"shaders/IsLp.frag.glsl", pps.toCString(), "r_"));

	PipelineInitializer init;

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
	m_r->createRenderTarget(
		m_r->getWidth(), m_r->getHeight(),
		RT_PIXEL_FORMAT, 1, SamplingFilter::LINEAR, MIPMAPS_COUNT, m_rt);

	FramebufferInitializer fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation =
		AttachmentLoadOperation::DONT_CARE;
	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	//
	// Create UBOs
	//
	m_pLightsBuffSize = m_maxPointLights * sizeof(ShaderPointLight);
	m_sLightsBuffSize = m_maxSpotLights * sizeof(ShaderSpotLight);

	for(U i = 0; i < m_pLightsBuffs.getSize(); ++i)
	{
		// Common variables
		m_commonVarsBuffs[i] = getGrManager().newInstance<Buffer>(
			sizeof(ShaderCommonUniforms), BufferUsageBit::STORAGE,
			BufferAccessBit::CLIENT_MAP_WRITE);

		// Point lights
		m_pLightsBuffs[i] = getGrManager().newInstance<Buffer>(
			m_pLightsBuffSize, BufferUsageBit::STORAGE,
			BufferAccessBit::CLIENT_MAP_WRITE);

		// Spot lights
		m_sLightsBuffs[i] = getGrManager().newInstance<Buffer>(
			m_sLightsBuffSize, BufferUsageBit::STORAGE,
			BufferAccessBit::CLIENT_MAP_WRITE);

		// Clusters
		m_clusterBuffers[i] = getGrManager().newInstance<Buffer>(
			m_r->getClusterCount() * sizeof(ShaderCluster),
			BufferUsageBit::STORAGE, BufferAccessBit::CLIENT_MAP_WRITE);

		// Index
		m_lightIdsBuffers[i] = getGrManager().newInstance<Buffer>(
			m_maxLightIds * sizeof(Lid),
			BufferUsageBit::STORAGE, BufferAccessBit::CLIENT_MAP_WRITE);
	}

	//
	// Create resource groups
	//
	for(U i = 0; i < m_rcGroups.getSize(); ++i)
	{
		ResourceGroupInitializer init;
		init.m_textures[0].m_texture = m_r->getMs().getRt0();
		init.m_textures[1].m_texture = m_r->getMs().getRt1();
		init.m_textures[2].m_texture = m_r->getMs().getRt2();
		init.m_textures[3].m_texture = m_r->getMs().getDepthRt();
		init.m_textures[4].m_texture = m_sm.getSpotTextureArray();
		init.m_textures[5].m_texture = m_sm.getOmniTextureArray();

		init.m_storageBuffers[0].m_buffer = m_commonVarsBuffs[i];
		init.m_storageBuffers[1].m_buffer = m_pLightsBuffs[i];
		init.m_storageBuffers[2].m_buffer = m_sLightsBuffs[i];
		init.m_storageBuffers[3].m_buffer = m_clusterBuffers[i];
		init.m_storageBuffers[4].m_buffer = m_lightIdsBuffers[i];

		m_rcGroups[i] = getGrManager().newInstance<ResourceGroup>(init);
	}

	//
	// Misc
	//
	ThreadPool& threadPool = m_r->getThreadPool();
	m_barrier = getAllocator().newInstance<Barrier>(
		threadPool.getThreadsCount());

	return ErrorCode::NONE;
}

//==============================================================================
Error Is::lightPass(CommandBufferPtr& cmdBuff)
{
	ThreadPool& threadPool = m_r->getThreadPool();
	m_cam = &m_r->getActiveCamera();
	FrustumComponent& fr = m_cam->getComponent<FrustumComponent>();
	VisibilityTestResults& vi = fr.getVisibilityTestResults();

	m_currentFrame = getGlobalTimestamp() % MAX_FRAMES_IN_FLIGHT;

	U clusterCount = m_r->getClusterCount();

	//
	// Quickly get the lights
	//
	U visiblePointLightsCount = 0;
	U visibleSpotLightsCount = 0;
	Array<SceneNode*, Sm::MAX_SHADOW_CASTERS> spotCasters;
	Array<SceneNode*, Sm::MAX_SHADOW_CASTERS> omniCasters;
	U spotCastersCount = 0;
	U omniCastersCount = 0;

	auto it = vi.getLightsBegin();
	auto lend = vi.getLightsEnd();
	for(; it != lend; ++it)
	{
		SceneNode* node = (*it).m_node;
		LightComponent& light = node->getComponent<LightComponent>();
		switch(light.getLightType())
		{
		case LightComponent::LightType::POINT:
			++visiblePointLightsCount;

			if(light.getShadowEnabled())
			{
				omniCasters[omniCastersCount++] = node;
			}
			break;
		case LightComponent::LightType::SPOT:
			++visibleSpotLightsCount;

			if(light.getShadowEnabled())
			{
				spotCasters[spotCastersCount++] = node;
			}
			break;
		default:
			ANKI_ASSERT(0);
			break;
		}
	}

	// Sanitize the counters
	visiblePointLightsCount = min<U>(visiblePointLightsCount, m_maxPointLights);
	visibleSpotLightsCount = min<U>(visibleSpotLightsCount, m_maxSpotLights);

	ANKI_COUNTER_INC(RENDERER_LIGHTS_COUNT,
		U64(visiblePointLightsCount + visibleSpotLightsCount));

	//
	// Do shadows pass
	//
	ANKI_CHECK(m_sm.run(
		{&spotCasters[0], spotCastersCount},
		{&omniCasters[0], omniCastersCount},
		cmdBuff));

	//
	// Write the lights and tiles UBOs
	//
	Array<WriteLightsTask, ThreadPool::MAX_THREADS> tasks;
	TaskCommonData taskData(getFrameAllocator());
	taskData.m_tempClusters.create(m_r->getClusterCount());

	if(visiblePointLightsCount)
	{
		void* data = m_pLightsBuffs[m_currentFrame]->map(
			0, visiblePointLightsCount * sizeof(ShaderPointLight),
			BufferAccessBit::CLIENT_MAP_WRITE);

		taskData.m_pointLights = SArray<ShaderPointLight>(
			static_cast<ShaderPointLight*>(data), visiblePointLightsCount);
	}

	if(visibleSpotLightsCount)
	{
		void* data = m_sLightsBuffs[m_currentFrame]->map(
			0, visibleSpotLightsCount * sizeof(ShaderSpotLight),
			BufferAccessBit::CLIENT_MAP_WRITE);

		taskData.m_spotLights = SArray<ShaderSpotLight>(
			static_cast<ShaderSpotLight*>(data), visibleSpotLightsCount);
	}

	taskData.m_lightsBegin = vi.getLightsBegin();
	taskData.m_lightsEnd = vi.getLightsEnd();

	taskData.m_is = this;

	// Map clusters
	void* data = m_clusterBuffers[m_currentFrame]->map(
		0,
		clusterCount * sizeof(ShaderCluster),
		BufferAccessBit::CLIENT_MAP_WRITE);

	taskData.m_clusters = SArray<ShaderCluster>(
		static_cast<ShaderCluster*>(data), clusterCount);

	// Map light IDs
	data = m_lightIdsBuffers[m_currentFrame]->map(
		0,
		m_maxLightIds * sizeof(Lid),
		BufferAccessBit::CLIENT_MAP_WRITE);

	taskData.m_lightIds = SArray<Lid>(static_cast<Lid*>(data), m_maxLightIds);

	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		tasks[i].m_data = &taskData;

		threadPool.assignNewTask(i, &tasks[i]);
	}

	// In the meantime set the state
	setState(cmdBuff);

	// Update uniforms
	updateCommonBlock(cmdBuff, fr);

	// Sync
	ANKI_CHECK(threadPool.waitForAllThreadsToFinish());

	//
	// Unmap
	//
	if(visiblePointLightsCount)
	{
		m_pLightsBuffs[m_currentFrame]->unmap();
	}

	if(visibleSpotLightsCount)
	{
		m_sLightsBuffs[m_currentFrame]->unmap();
	}

	m_clusterBuffers[m_currentFrame]->unmap();
	m_lightIdsBuffers[m_currentFrame]->unmap();

	//
	// Draw
	//
	cmdBuff->drawArrays(4, m_r->getTileCount());

	return ErrorCode::NONE;
}

//==============================================================================
void Is::binLights(U32 threadId, PtrSize threadsCount, TaskCommonData& task)
{
	U lightsCount = task.m_lightsEnd - task.m_lightsBegin;
	const FrustumComponent& camfrc = m_cam->getComponent<FrustumComponent>();
	const MoveComponent& cammove = m_cam->getComponent<MoveComponent>();

	// Iterate lights and bin them
	PtrSize start, end;
	ThreadPool::Task::choseStartEnd(
		threadId, threadsCount, lightsCount, start, end);

	ClustererTestResult testResult;
	m_r->getClusterer().initTestResults(getFrameAllocator(), testResult);

	for(U64 i = start; i < end; i++)
	{
		SceneNode* snode = (*(task.m_lightsBegin + i)).m_node;
		MoveComponent& move = snode->getComponent<MoveComponent>();
		LightComponent& light = snode->getComponent<LightComponent>();
		SpatialComponent& sp = snode->getComponent<SpatialComponent>();

		switch(light.getLightType())
		{
		case LightComponent::LightType::POINT:
			{
				I pos = writePointLight(light, move, camfrc, task);
				if(pos != -1)
				{
					binLight(sp, pos, 0, task, testResult);
				}
			}
			break;
		case LightComponent::LightType::SPOT:
			{
				const FrustumComponent* frc =
					snode->tryGetComponent<FrustumComponent>();
				I pos = writeSpotLight(light, move, frc, cammove, camfrc, task);
				if(pos != -1)
				{
					binLight(sp, pos, 1, task, testResult);
				}
			}
			break;
		default:
			ANKI_ASSERT(0);
			break;
		}
	}

	//
	// Last thing, update the real clusters
	//
	m_barrier->wait();

	U clusterCount = m_r->getClusterCount();

	ThreadPool::Task::choseStartEnd(
		threadId, threadsCount, clusterCount, start, end);

	// Run per tile
	for(U i = start; i < end; ++i)
	{
		auto& cluster = task.m_tempClusters[i];

		const U countP = cluster.m_pointCount.load();
		const U countS = cluster.m_spotCount.load();
		const U count = countP + countS;

		auto& c = task.m_clusters[i];
		c.m_combo = 0;

		// Early exit
		if(ANKI_UNLIKELY(count == 0))
		{
			continue;
		}

		// Check if the previous cluster contains the same lights as this one
		// and if yes then merge them. This will avoid allocating new IDs (and
		// thrashing GPU caches).
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

		const U offset = task.m_lightIdsCount.fetchAdd(count);

		if(offset + count <= m_maxLightIds)
		{
			ANKI_ASSERT(offset <= 0xFFFF);
			c.m_combo = offset << 16;

			if(countP > 0)
			{
				ANKI_ASSERT(countP <= 0xFF);
				c.m_combo |= countP << 8;
				memcpy(&task.m_lightIds[offset], &cluster.m_pointIds[0],
					countP * sizeof(Lid));
			}

			if(countS > 0)
			{
				ANKI_ASSERT(countS <= 0xFF);
				c.m_combo |= countS;
				memcpy(&task.m_lightIds[offset + countP], &cluster.m_spotIds[0],
					countS * sizeof(Lid));
			}
		}
		else
		{
			ANKI_LOGW("Light IDs buffer too small");
		}
	}
}

//==============================================================================
I Is::writePointLight(const LightComponent& lightc,
	const MoveComponent& lightMove,
	const FrustumComponent& camFrc, TaskCommonData& task)
{
	// Get GPU light
	I i = task.m_pointLightsCount.fetchAdd(1);
	if(ANKI_UNLIKELY(i >= I(m_maxPointLights)))
	{
		return -1;
	}

	ShaderPointLight& slight = task.m_pointLights[i];

	Vec4 pos = camFrc.getViewMatrix()
		* lightMove.getWorldTransform().getOrigin().xyz1();

	slight.m_posRadius = Vec4(pos.xyz(), -1.0 / lightc.getRadius());
	slight.m_diffuseColorShadowmapId = lightc.getDiffuseColor();

	if(!lightc.getShadowEnabled())
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
	const MoveComponent& lightMove, const FrustumComponent* lightFrc,
	const MoveComponent& camMove, const FrustumComponent& camFrc,
	TaskCommonData& task)
{
	I i = task.m_spotLightsCount.fetchAdd(1);
	if(ANKI_UNLIKELY(i >= I(m_maxSpotTexLights)))
	{
		return -1;
	}

	ShaderSpotLight& light = task.m_spotLights[i];
	F32 shadowmapIndex = INVALID_TEXTURE_INDEX;

	if(lightc.getShadowEnabled())
	{
		// Write matrix
		static const Mat4 biasMat4(
			0.5, 0.0, 0.0, 0.5,
			0.0, 0.5, 0.0, 0.5,
			0.0, 0.0, 0.5, 0.5,
			0.0, 0.0, 0.0, 1.0);
		// bias * proj_l * view_l * world_c
		light.m_texProjectionMat =
			biasMat4
			* lightFrc->getViewProjectionMatrix()
			* Mat4(camMove.getWorldTransform());

		// Transpose because of driver bug
		light.m_texProjectionMat.transpose();

		shadowmapIndex = (F32)lightc.getShadowMapIndex();
	}

	// Pos & dist
	Vec4 pos =
		camFrc.getViewMatrix()
		* lightMove.getWorldTransform().getOrigin().xyz1();
	light.m_posRadius = Vec4(pos.xyz(), -1.0 / lightc.getDistance());

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
	light.m_outerCosInnerCos = Vec4(
		lightc.getOuterAngleCos(),
		lightc.getInnerAngleCos(),
		1.0,
		1.0);

	return i;
}

//==============================================================================
void Is::binLight(
	SpatialComponent& sp,
	U pos,
	U lightType,
	TaskCommonData& task,
	ClustererTestResult& testResult)
{
	m_r->getClusterer().bin(sp.getSpatialCollisionShape(), sp.getAabb(),
		testResult);

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
			cluster.m_pointIds[i] = pos;
			break;
		case 1:
			i = cluster.m_spotCount.fetchAdd(1) % MAX_TYPED_LIGHTS_PER_CLUSTER;
			cluster.m_spotIds[i] = pos;
			break;
		default:
			ANKI_ASSERT(0);
		}
	}
}

//==============================================================================
void Is::setState(CommandBufferPtr& cmdBuff)
{
	cmdBuff->bindFramebuffer(m_fb);
	cmdBuff->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	cmdBuff->bindPipeline(m_lightPpline);
	cmdBuff->bindResourceGroup(m_rcGroups[m_currentFrame], 0);
}

//==============================================================================
Error Is::run(CommandBufferPtr& cmdBuff)
{
	// Do the light pass including the shadow passes
	return lightPass(cmdBuff);
}

//==============================================================================
void Is::updateCommonBlock(CommandBufferPtr& cmdb, const FrustumComponent& fr)
{
	ShaderCommonUniforms* blk =
		static_cast<ShaderCommonUniforms*>(
			m_commonVarsBuffs[m_currentFrame]->map(
			0, sizeof(ShaderCommonUniforms),
			BufferAccessBit::CLIENT_MAP_WRITE));

	// Start writing
	blk->m_projectionParams = m_r->getProjectionParameters();
	blk->m_sceneAmbientColor = m_ambientColor;
	blk->m_viewMat = fr.getViewMatrix().getTransposed();
	blk->m_nearFarClustererDivisor = Vec4(
		fr.getFrustum().getNear(),
		fr.getFrustum().getFar(),
		m_r->getClusterer().getDivisor(),
		0.0);

	Vec3 groundLightDir;
	if(m_groundLightEnabled)
	{
		const Mat4& viewMat =
			m_cam->getComponent<FrustumComponent>().getViewMatrix();
		blk->m_groundLightDir = Vec4(-viewMat.getColumn(1).xyz(), 1.0);
	}

	blk->m_tileCount = UVec4(m_r->getTileCountXY(), m_r->getTileCount(), 0);

	m_commonVarsBuffs[m_currentFrame]->unmap();
}

} // end namespace anki
