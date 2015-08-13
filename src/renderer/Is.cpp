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

//==============================================================================
// Shader structs and block representations. All positions and directions in
// viewspace
// For documentation see the shaders

namespace shader {

struct Tile
{
	U32 m_offset;
	U32 m_pointLightsCount;
	U32 m_spotLightsCount;
	U32 m_spotTexLightsCount;
};

struct Light
{
	Vec4 m_posRadius;
	Vec4 m_diffuseColorShadowmapId;
	Vec4 m_specularColorTexId;
};

struct PointLight: Light
{};

struct SpotLight: Light
{
	Vec4 m_lightDir;
	Vec4 m_outerCosInnerCos;
	Mat4 m_texProjectionMat; ///< Texture projection matrix
};

struct CommonUniforms
{
	Vec4 m_projectionParams;
	Vec4 m_sceneAmbientColor;
	Vec4 m_groundLightDir;
	Mat4 m_viewMat;
};

} // end namespace shader

//==============================================================================

using Lid = U32; ///< Light ID type
static const U MAX_TYPED_LIGHTS_PER_TILE = 16;

class TileData
{
public:
	Atomic<U32> m_pointCount;
	Atomic<U32> m_spotCount;
	Atomic<U32> m_spotTexCount;

	Array<Lid, MAX_TYPED_LIGHTS_PER_TILE> m_pointIds;
	Array<Lid, MAX_TYPED_LIGHTS_PER_TILE> m_spotIds;
	Array<Lid, MAX_TYPED_LIGHTS_PER_TILE> m_spotTexIds;
};

/// Common data for all tasks.
class TaskCommonData
{
public:
	// To fill the light buffers
	SArray<shader::PointLight> m_pointLights;
	SArray<shader::SpotLight> m_spotLights;
	SArray<shader::SpotLight> m_spotTexLights;

	SArray<Lid> m_lightIds;
	SArray<shader::Tile> m_tiles;

	Atomic<U32> m_pointLightsCount;
	Atomic<U32> m_spotLightsCount;
	Atomic<U32> m_spotTexLightsCount;

	// To fill the tile buffers
	Array2d<TileData, ANKI_RENDERER_MAX_TILES_Y, ANKI_RENDERER_MAX_TILES_X>
		m_tempTiles;

	// To fill the light index buffer
	Atomic<U32> m_lightIdsCount;

	// Misc
	VisibilityTestResults::Container::ConstIterator m_lightsBegin = nullptr;
	VisibilityTestResults::Container::ConstIterator m_lightsEnd = nullptr;

	WeakPtr<Barrier> m_barrier;
	Is* m_is = nullptr;
};

/// Write the lights to the GPU buffers.
class WriteLightsTask: public Threadpool::Task
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
		getAllocator().deleteInstance(&m_barrier.get());
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

	m_maxLightIds = config.getNumber("is.maxLightsPerTile");

	if(m_maxLightIds == 0)
	{
		ANKI_LOGE("Incorrect number of max light indices");
		return ErrorCode::USER_DATA;
	}

	m_maxLightIds *= m_r->getTilesCountXY();

	//
	// Init the passes
	//
	ANKI_CHECK(m_sm.init(config));

	//
	// Load the programs
	//
	StringAuto pps(getAllocator());

	pps.sprintf(
		"\n#define TILES_X_COUNT %u\n"
		"#define TILES_Y_COUNT %u\n"
		"#define TILES_COUNT %u\n"
		"#define RENDERER_WIDTH %u\n"
		"#define RENDERER_HEIGHT %u\n"
		"#define MAX_POINT_LIGHTS %u\n"
		"#define MAX_SPOT_LIGHTS %u\n"
		"#define MAX_SPOT_TEX_LIGHTS %u\n"
		"#define MAX_LIGHT_INDICES %u\n"
		"#define GROUND_LIGHT %u\n"
		"#define TILES_BLOCK_BINDING %u\n"
		"#define POISSON %u\n",
		m_r->getTilesCount().x(),
		m_r->getTilesCount().y(),
		(m_r->getTilesCount().x() * m_r->getTilesCount().y()),
		m_r->getWidth(),
		m_r->getHeight(),
		m_maxPointLights,
		m_maxSpotLights,
		m_maxSpotTexLights,
		m_maxLightIds,
		m_groundLightEnabled,
		TILES_BLOCK_BINDING,
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
	m_pLightsBuffSize = m_maxPointLights * sizeof(shader::PointLight);
	m_sLightsBuffSize = m_maxSpotLights * sizeof(shader::SpotLight);
	m_stLightsBuffSize = m_maxSpotTexLights * sizeof(shader::SpotLight);

	for(U i = 0; i < m_pLightsBuffs.getSize(); ++i)
	{
		// Point lights
		m_pLightsBuffs[i] = getGrManager().newInstance<Buffer>(
			m_pLightsBuffSize, BufferUsageBit::STORAGE,
			BufferAccessBit::CLIENT_MAP_WRITE);

		// Spot lights
		m_sLightsBuffs[i] = getGrManager().newInstance<Buffer>(
			m_sLightsBuffSize, BufferUsageBit::STORAGE,
			BufferAccessBit::CLIENT_MAP_WRITE);

		// Spot tex lights
		m_stLightsBuffs[i] = getGrManager().newInstance<Buffer>(
			m_stLightsBuffSize, BufferUsageBit::STORAGE,
			BufferAccessBit::CLIENT_MAP_WRITE);

		// Tiles
		m_tilesBuffers[i] = getGrManager().newInstance<Buffer>(
			m_r->getTilesCountXY() * sizeof(shader::Tile),
			BufferUsageBit::STORAGE, BufferAccessBit::CLIENT_MAP_WRITE);

		// Index
		m_lightIdsBuffers[i] = getGrManager().newInstance<Buffer>(
			(m_maxPointLights * m_maxSpotLights * m_maxSpotTexLights)
			* sizeof(U32),
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

		init.m_storageBuffers[0].m_buffer = m_pLightsBuffs[i];
		init.m_storageBuffers[1].m_buffer = m_sLightsBuffs[i];
		init.m_storageBuffers[2].m_buffer = m_stLightsBuffs[i];
		init.m_storageBuffers[3].m_buffer = m_tilesBuffers[i];
		init.m_storageBuffers[4].m_buffer = m_lightIdsBuffers[i];

		m_rcGroups[i] = getGrManager().newInstance<ResourceGroup>(init);
	}

	//
	// Misc
	//
	Threadpool& threadPool = m_r->getThreadpool();
	m_barrier = getAllocator().newInstance<Barrier>(
		threadPool.getThreadsCount());

	return ErrorCode::NONE;
}

//==============================================================================
Error Is::lightPass(CommandBufferPtr& cmdBuff)
{
	Threadpool& threadPool = m_r->getThreadpool();
	m_cam = &m_r->getActiveCamera();
	FrustumComponent& fr = m_cam->getComponent<FrustumComponent>();
	VisibilityTestResults& vi = fr.getVisibilityTestResults();

	m_currentFrame = getGlobalTimestamp() % MAX_FRAMES_IN_FLIGHT;

	U tilesCount = m_r->getTilesCountXY();

	//
	// Quickly get the lights
	//
	U visiblePointLightsCount = 0;
	U visibleSpotLightsCount = 0;
	U visibleSpotTexLightsCount = 0;
	Array<SceneNode*, Sm::MAX_SHADOW_CASTERS> shadowCasters;
	Array<SceneNode*, Sm::MAX_SHADOW_CASTERS> omniCasters;
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
			{
				if(light.getShadowEnabled())
				{
					shadowCasters[visibleSpotTexLightsCount++] = node;
				}
				else
				{
					++visibleSpotLightsCount;
				}
				break;
			}
		default:
			ANKI_ASSERT(0);
			break;
		}
	}

	// Sanitize the counters
	visiblePointLightsCount = min<U>(visiblePointLightsCount, m_maxPointLights);
	visibleSpotLightsCount = min<U>(visibleSpotLightsCount, m_maxSpotLights);
	visibleSpotTexLightsCount =
		min<U>(visibleSpotTexLightsCount, m_maxSpotTexLights);

	ANKI_COUNTER_INC(RENDERER_LIGHTS_COUNT,
		U64(visiblePointLightsCount + visibleSpotLightsCount
		+ visibleSpotTexLightsCount));

	//
	// Do shadows pass
	//
	ANKI_CHECK(m_sm.run(
		{&shadowCasters[0], visibleSpotTexLightsCount},
		{&omniCasters[0], omniCastersCount},
		cmdBuff));

	//
	// Write the lights and tiles UBOs
	//
	Array<WriteLightsTask, Threadpool::MAX_THREADS> tasks;
	TaskCommonData taskData;
	memset(&taskData, 0, sizeof(taskData));

	if(visiblePointLightsCount)
	{
		void* data = m_pLightsBuffs[m_currentFrame]->map(
			0, visiblePointLightsCount * sizeof(shader::PointLight),
			BufferAccessBit::CLIENT_MAP_WRITE);

		taskData.m_pointLights = SArray<shader::PointLight>(
			static_cast<shader::PointLight*>(data), visiblePointLightsCount);
	}

	if(visibleSpotLightsCount)
	{
		void* data = m_sLightsBuffs[m_currentFrame]->map(
			0, visibleSpotLightsCount * sizeof(shader::SpotLight),
			BufferAccessBit::CLIENT_MAP_WRITE);

		taskData.m_spotLights = SArray<shader::SpotLight>(
			static_cast<shader::SpotLight*>(data), visibleSpotLightsCount);
	}

	if(visibleSpotTexLightsCount)
	{
		void* data = m_stLightsBuffs[m_currentFrame]->map(
			0, visibleSpotTexLightsCount * sizeof(shader::SpotLight),
			BufferAccessBit::CLIENT_MAP_WRITE);

		taskData.m_spotTexLights = SArray<shader::SpotLight>(
			static_cast<shader::SpotLight*>(data), visibleSpotTexLightsCount);
	}

	taskData.m_lightsBegin = vi.getLightsBegin();
	taskData.m_lightsEnd = vi.getLightsEnd();

	taskData.m_barrier = m_barrier;

	taskData.m_is = this;

	// Map tiles
	void* data = m_tilesBuffers[m_currentFrame]->map(
		0,
		tilesCount * sizeof(shader::Tile),
		BufferAccessBit::CLIENT_MAP_WRITE);

	taskData.m_tiles = SArray<shader::Tile>(
		static_cast<shader::Tile*>(data), tilesCount);

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

	if(visibleSpotTexLightsCount)
	{
		m_stLightsBuffs[m_currentFrame]->unmap();
	}

	m_tilesBuffers[m_currentFrame]->unmap();
	m_lightIdsBuffers[m_currentFrame]->unmap();

	//
	// Draw
	//
	cmdBuff->drawArrays(4, m_r->getTilesCountXY());

	return ErrorCode::NONE;
}

//==============================================================================
void Is::binLights(U32 threadId, PtrSize threadsCount, TaskCommonData& task)
{
	U ligthsCount = task.m_lightsEnd - task.m_lightsBegin;
	const FrustumComponent& camfrc = m_cam->getComponent<FrustumComponent>();
	const MoveComponent& cammove = m_cam->getComponent<MoveComponent>();

	// Iterate lights and bin them
	PtrSize start, end;
	Threadpool::Task::choseStartEnd(
		threadId, threadsCount, ligthsCount, start, end);

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
					binLight(sp, pos, 0, task);
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
					binLight(sp, pos, light.getShadowEnabled() ? 2 : 1, task);
				}
			}
			break;
		default:
			ANKI_ASSERT(0);
			break;
		}
	}

	//
	// Last thing, update the real tiles
	//
	task.m_barrier->wait();

	const UVec2 tilesCount2d = m_r->getTilesCount();
	U tilesCount = tilesCount2d.x() * tilesCount2d.y();

	Threadpool::Task::choseStartEnd(
		threadId, threadsCount, tilesCount, start, end);

	// Run per tile
	for(U i = start; i < end; ++i)
	{
		U y = i / tilesCount2d.x();
		U x = i % tilesCount2d.x();
		const auto& tile = task.m_tempTiles[y][x];

		const U countP = tile.m_pointCount.load();
		const U countS = tile.m_spotCount.load();
		const U countST = tile.m_spotTexCount.load();
		const U count = countP + countS + countST;

		const U offset = task.m_lightIdsCount.fetchAdd(count);

		auto& t = task.m_tiles[i];

		if(offset + count <= m_maxLightIds)
		{
			t.m_offset = offset;

			if(countP > 0)
			{
				t.m_pointLightsCount = countP;
				memcpy(&task.m_lightIds[offset], &tile.m_pointIds[0],
					countP * sizeof(Lid));
			}
			else
			{
				t.m_pointLightsCount = 0;
			}

			if(countS > 0)
			{
				t.m_spotLightsCount = countS;
				memcpy(&task.m_lightIds[offset + countP], &tile.m_spotIds[0],
					countS * sizeof(Lid));
			}
			else
			{
				t.m_spotLightsCount = 0;
			}

			if(countST > 0)
			{
				t.m_spotTexLightsCount = countST;
				memcpy(&task.m_lightIds[offset + countP + countS],
					&tile.m_spotTexIds[0], countST * sizeof(Lid));
			}
			else
			{
				t.m_spotTexLightsCount = 0;
			}
		}
		else
		{
			memset(&t, 0, sizeof(t));

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

	shader::PointLight& slight = task.m_pointLights[i];

	Vec4 pos = camFrc.getViewMatrix()
		* lightMove.getWorldTransform().getOrigin().xyz1();

	slight.m_posRadius = Vec4(pos.xyz(), -1.0 / lightc.getRadius());
	slight.m_diffuseColorShadowmapId = lightc.getDiffuseColor();

	if(!lightc.getShadowEnabled())
	{
		slight.m_diffuseColorShadowmapId.w() = 128.0;
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
	Bool isTexLight = lightc.getShadowEnabled();
	I i;
	shader::SpotLight* baseslight = nullptr;

	if(isTexLight)
	{
		// Spot tex light

		i = task.m_spotTexLightsCount.fetchAdd(1);
		if(ANKI_UNLIKELY(i >= I(m_maxSpotTexLights)))
		{
			return -1;
		}

		shader::SpotLight& slight = task.m_spotTexLights[i];
		baseslight = &slight;

		// Write matrix
		static const Mat4 biasMat4(
			0.5, 0.0, 0.0, 0.5,
			0.0, 0.5, 0.0, 0.5,
			0.0, 0.0, 0.5, 0.5,
			0.0, 0.0, 0.0, 1.0);
		// bias * proj_l * view_l * world_c
		slight.m_texProjectionMat =
			biasMat4
			* lightFrc->getViewProjectionMatrix()
			* Mat4(camMove.getWorldTransform());

		// Transpose because of driver bug
		slight.m_texProjectionMat.transpose();
	}
	else
	{
		// Spot light without texture

		i = task.m_spotLightsCount.fetchAdd(1);
		if(ANKI_UNLIKELY(i >= I(m_maxSpotLights)))
		{
			return -1;
		}

		shader::SpotLight& slight = task.m_spotLights[i];
		baseslight = &slight;
	}

	// Write common stuff
	ANKI_ASSERT(baseslight);

	// Pos & dist
	Vec4 pos =
		camFrc.getViewMatrix()
		* lightMove.getWorldTransform().getOrigin().xyz1();
	baseslight->m_posRadius = Vec4(pos.xyz(), -1.0 / lightc.getDistance());

	// Diff color and shadowmap ID now
	baseslight->m_diffuseColorShadowmapId =
		Vec4(lightc.getDiffuseColor().xyz(), (F32)lightc.getShadowMapIndex());

	// Spec color
	baseslight->m_specularColorTexId = lightc.getSpecularColor();

	// Light dir
	Vec3 lightDir = -lightMove.getWorldTransform().getRotation().getZAxis();
	lightDir = camFrc.getViewMatrix().getRotationPart() * lightDir;
	baseslight->m_lightDir = Vec4(lightDir, 0.0);

	// Angles
	baseslight->m_outerCosInnerCos = Vec4(
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
	TaskCommonData& task)
{
	// Do the tests
	Tiler::TestResult visTiles;
	Tiler::TestParameters params;
	params.m_collisionShape = &sp.getSpatialCollisionShape();
	params.m_collisionShapeBox = &sp.getAabb();
	params.m_nearPlane = true;
	params.m_output = &visTiles;
	m_r->getTiler().test(params);

	// Bin to the correct tiles
	for(U t = 0; t < visTiles.m_count; t++)
	{
		U x = visTiles.m_tileIds[t].m_x;
		U y = visTiles.m_tileIds[t].m_y;

		auto& tile = task.m_tempTiles[y][x];
		U idx;

		switch(lightType)
		{
		case 0:
			idx = tile.m_pointCount.fetchAdd(1) % MAX_TYPED_LIGHTS_PER_TILE;
			tile.m_pointIds[idx] = pos;
			break;
		case 1:
			idx = tile.m_spotCount.fetchAdd(1) % MAX_TYPED_LIGHTS_PER_TILE;
			tile.m_spotIds[idx] = pos;
			break;
		case 2:
			idx = tile.m_spotTexCount.fetchAdd(1) % MAX_TYPED_LIGHTS_PER_TILE;
			tile.m_spotTexIds[idx] = pos;
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
	cmdBuff->bindResourceGroup(m_rcGroups[m_currentFrame]);
}

//==============================================================================
Error Is::run(CommandBufferPtr& cmdBuff)
{
	// Do the light pass including the shadow passes
	return lightPass(cmdBuff);
}

//==============================================================================
void Is::updateCommonBlock(CommandBufferPtr& cmdb, FrustumComponent& fr)
{
	shader::CommonUniforms* blk;
	cmdb->updateDynamicUniforms(sizeof(*blk), blk);

	// Start writing
	blk->m_projectionParams = m_r->getProjectionParameters();
	blk->m_sceneAmbientColor = m_ambientColor;
	blk->m_viewMat = fr.getViewMatrix().getTransposed();

	Vec3 groundLightDir;
	if(m_groundLightEnabled)
	{
		const Mat4& viewMat =
			m_cam->getComponent<FrustumComponent>().getViewMatrix();
		blk->m_groundLightDir = Vec4(-viewMat.getColumn(1).xyz(), 1.0);
	}
}

} // end namespace anki
