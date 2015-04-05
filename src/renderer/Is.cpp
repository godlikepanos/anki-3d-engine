// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Is.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Light.h"
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
};

} // end namespace shader

//==============================================================================

using Lid = U32; ///< Light ID type
static const U MAX_TYPED_LIGHTS_PER_TILE = 16;

struct TileData
{
	Atomic<U32> m_pointCount;
	Atomic<U32> m_spotCount;
	Atomic<U32> m_spotTexCount;

	Array<Lid, MAX_TYPED_LIGHTS_PER_TILE> m_pointIds;
	Array<Lid, MAX_TYPED_LIGHTS_PER_TILE> m_spotIds;
	Array<Lid, MAX_TYPED_LIGHTS_PER_TILE> m_spotTexIds;
};

/// Common data for all tasks.
struct TaskCommonData
{
	// To fill the light buffers
	SArray<shader::PointLight> m_pointLights;
	SArray<shader::SpotLight> m_spotLights;
	SArray<shader::SpotLight> m_spotTexLights;

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

	Barrier* m_barrier = nullptr;
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
Is::Is(Renderer* r)
:	RenderingPass(r), 
	m_sm(r)
{}

//==============================================================================
Is::~Is()
{}

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
	Error err = ErrorCode::NONE;

	m_groundLightEnabled = config.get("is.groundLightEnabled");
	m_maxPointLights = config.get("is.maxPointLights");
	m_maxSpotLights = config.get("is.maxSpotLights");
	m_maxSpotTexLights = config.get("is.maxSpotTexLights");

	if(m_maxPointLights < 1 || m_maxSpotLights < 1 || m_maxSpotTexLights < 1)
	{
		ANKI_LOGE("Incorrect number of max lights");
		return ErrorCode::USER_DATA;
	}

	m_maxLightIds = config.get("is.maxLightsPerTile");

	if(m_maxLightIds == 0)
	{
		ANKI_LOGE("Incorrect number of max light indices");
		return ErrorCode::USER_DATA;
	}
	
	m_maxLightIds *= m_r->getTilesCountXY();


	//
	// Init the passes
	//
	err = m_sm.init(config);
	if(err) return err;

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
	CommandBufferHandle cmdBuff;
	ANKI_CHECK(cmdBuff.create(&getGrManager())); // Job for initialization

	ANKI_CHECK(m_lightVert.loadToCache(&getResourceManager(),
		"shaders/IsLp.vert.glsl", pps.toCString(), "r_"));

	ANKI_CHECK(m_lightFrag.loadToCache(&getResourceManager(),
		"shaders/IsLp.frag.glsl", pps.toCString(), "r_"));

	PipelineHandle::Initializer init;
		init.m_shaders[U(ShaderType::VERTEX)] = m_lightVert->getGrShader();
		init.m_shaders[U(ShaderType::FRAGMENT)] = m_lightFrag->getGrShader();
	ANKI_CHECK(m_lightPpline.create(cmdBuff, init));

	//
	// Create framebuffer
	//

	ANKI_CHECK(m_r->createRenderTarget(
		m_r->getWidth(), m_r->getHeight(), GL_RGB8, 1, m_rt));

	FramebufferHandle::Initializer fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	ANKI_CHECK(m_fb.create(cmdBuff, fbInit));

	//
	// Init the quad
	//
	static const F32 quadVertCoords[][2] = 
		{{1.0, 1.0}, {0.0, 1.0}, {1.0, 0.0}, {0.0, 0.0}};

	ANKI_CHECK(m_quadPositionsVertBuff.create(cmdBuff, GL_ARRAY_BUFFER, 
		&quadVertCoords[0][0], sizeof(quadVertCoords), 0));

	//
	// Create UBOs
	//
	ANKI_CHECK(m_commonBuffer.create(cmdBuff, GL_UNIFORM_BUFFER, nullptr,
		sizeof(shader::CommonUniforms), GL_DYNAMIC_STORAGE_BIT));

	for(U i = 0; i < MAX_FRAMES; ++i)
	{
		// Lights
		ANKI_CHECK(m_lightsBuffers[i].create(cmdBuff, GL_SHADER_STORAGE_BUFFER, 
			nullptr, calcLightsBufferSize(), 
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));

		// Tiles
		ANKI_CHECK(m_tilesBuffers[i].create(cmdBuff, GL_SHADER_STORAGE_BUFFER,
			nullptr, m_r->getTilesCountXY() * sizeof(shader::Tile),
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));

		// Index
		ANKI_CHECK(m_lightIdsBuffers[i].create(cmdBuff, 
			GL_SHADER_STORAGE_BUFFER,
			nullptr, (m_maxPointLights * m_maxSpotLights * m_maxSpotTexLights)
			* sizeof(U32),
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));
	}

	// Flush
	cmdBuff.flush();

	// Get addresses
	for(U i = 0; i < MAX_FRAMES; ++i)
	{
		m_lightsBufferAddresses[i] =
			m_lightsBuffers[i].getPersistentMappingAddress();

		m_tilesBufferAddresses[i] = 
			m_tilesBuffers[i].getPersistentMappingAddress();

		m_lightIdsBufferAddresses[i] = 
			m_lightIdsBuffers[i].getPersistentMappingAddress();
	}

	return err;
}

//==============================================================================
Error Is::lightPass(CommandBufferHandle& cmdBuff)
{
	Error err = ErrorCode::NONE;
	Threadpool& threadPool = m_r->_getThreadpool();
	m_cam = &m_r->getSceneGraph().getActiveCamera();
	FrustumComponent& fr = m_cam->getComponent<FrustumComponent>();
	VisibilityTestResults& vi = fr.getVisibilityTestResults();

	m_currentFrame = getGlobalTimestamp() % MAX_FRAMES;

	//
	// Quickly get the lights
	//
	U visiblePointLightsCount = 0;
	U visibleSpotLightsCount = 0;
	U visibleSpotTexLightsCount = 0;
	Array<SceneNode*, Sm::MAX_SHADOW_CASTERS> shadowCasters;

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
	ANKI_CHECK(m_sm.run(&shadowCasters[0], visibleSpotTexLightsCount, cmdBuff));

	//
	// Write the lights and tiles UBOs
	//
	U32 blockAlignment = 
		getGrManager().getBufferOffsetAlignment(m_lightsBuffers[0].getTarget());

	// Get the offsets and sizes of each uniform block
	PtrSize pointLightsOffset = 0;
	PtrSize pointLightsSize = getAlignedRoundUp(
		blockAlignment, sizeof(shader::PointLight) * visiblePointLightsCount);

	PtrSize spotLightsOffset = pointLightsSize;
	PtrSize spotLightsSize = getAlignedRoundUp(
		blockAlignment, sizeof(shader::SpotLight) * visibleSpotLightsCount);

	PtrSize spotTexLightsOffset = spotLightsOffset + spotLightsSize;
	PtrSize spotTexLightsSize = getAlignedRoundUp(blockAlignment, 
		sizeof(shader::SpotLight) * visibleSpotTexLightsCount);

	ANKI_ASSERT(
		spotTexLightsOffset + spotTexLightsSize <= calcLightsBufferSize());

	// Fire the super cmdBuff
	Array<WriteLightsTask, Threadpool::MAX_THREADS> tasks;
	TaskCommonData taskData;
	memset(&taskData, 0, sizeof(taskData));

	U8* lightsBase = static_cast<U8*>(m_lightsBufferAddresses[m_currentFrame]);
	if(visiblePointLightsCount)
	{
		taskData.m_pointLights = SArray<shader::PointLight>(
			lightsBase + pointLightsOffset, visiblePointLightsCount);
	}

	if(visibleSpotLightsCount)
	{
		taskData.m_spotLights = SArray<shader::SpotLight>(
			lightsBase + spotLightsOffset, visibleSpotLightsCount);
	}

	if(visibleSpotTexLightsCount) 
	{
		taskData.m_spotTexLights = SArray<shader::SpotLight>(
			lightsBase + spotTexLightsOffset, visibleSpotTexLightsCount);
	}

	taskData.m_lightsBegin = vi.getLightsBegin();
	taskData.m_lightsEnd = vi.getLightsEnd();

	Barrier barrier(threadPool.getThreadsCount());
	taskData.m_barrier = &barrier;

	taskData.m_is = this;

	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		tasks[i].m_data = &taskData;

		threadPool.assignNewTask(i, &tasks[i]);
	}

	// In the meantime set the state
	setState(cmdBuff);

	// Sync
	ANKI_CHECK(threadPool.waitForAllThreadsToFinish());

	//
	// Setup uniforms
	//

	// shader prog
	ANKI_CHECK(updateCommonBlock(cmdBuff));

	m_commonBuffer.bindShaderBuffer(cmdBuff, COMMON_UNIFORMS_BLOCK_BINDING);

	if(pointLightsSize > 0)
	{
		m_lightsBuffers[m_currentFrame].bindShaderBuffer(cmdBuff, 
			pointLightsOffset, pointLightsSize, POINT_LIGHTS_BLOCK_BINDING);
	}

	if(spotLightsSize > 0)
	{
		m_lightsBuffers[m_currentFrame].bindShaderBuffer(cmdBuff, 
			spotLightsOffset, spotLightsSize, SPOT_LIGHTS_BLOCK_BINDING);
	}

	if(spotTexLightsSize > 0)
	{
		m_lightsBuffers[m_currentFrame].bindShaderBuffer(cmdBuff,
			spotTexLightsOffset, spotTexLightsSize, 
			SPOT_TEX_LIGHTS_BLOCK_BINDING);
	}

	m_tilesBuffers[m_currentFrame].bindShaderBuffer(
		cmdBuff, TILES_BLOCK_BINDING);

	m_lightIdsBuffers[m_currentFrame].bindShaderBuffer(
		cmdBuff, LIGHT_IDS_BLOCK_BINDING);

	// The binding points should much the shader
	Array<TextureHandle, 4> tarr = {{
		m_r->getMs()._getRt0(), 
		m_r->getMs()._getRt1(), 
		m_r->getMs()._getDepthRt(),
		m_sm.m_sm2DArrayTex}};

	cmdBuff.bindTextures(0, tarr.begin(), tarr.getSize());

	//
	// Draw
	//

	m_lightPpline.bind(cmdBuff);

	m_quadPositionsVertBuff.bindVertexBuffer(cmdBuff, 
		2, GL_FLOAT, false, 0, 0, 0);

	cmdBuff.drawArrays(GL_TRIANGLE_STRIP, 4, m_r->getTilesCountXY());

	return err;
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

	SArray<shader::Tile> stiles(
		m_tilesBufferAddresses[m_currentFrame],
		tilesCount);

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

		auto& t = stiles[i];

		if(offset + count <= m_maxLightIds)
		{
			SArray<Lid> lightIds(
				m_lightIdsBufferAddresses[m_currentFrame],
				m_maxLightIds);

			t.m_offset = offset;

			if(countP > 0)
			{
				t.m_pointLightsCount = countP;
				memcpy(&lightIds[offset], &tile.m_pointIds[0], 
					countP * sizeof(Lid));
			}
			else
			{
				t.m_pointLightsCount = 0;
			}
			
			if(countS > 0)
			{
				t.m_spotLightsCount = countS;
				memcpy(&lightIds[offset + countP], &tile.m_spotIds[0], 
					countS * sizeof(Lid));
			}
			else
			{
				t.m_spotLightsCount = 0;
			}

			if(countST > 0)
			{
				t.m_spotTexLightsCount = countST;
				memcpy(&lightIds[offset + countP + countS], 
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

	// extend points
	const PerspectiveFrustum& frustum = 
		static_cast<const PerspectiveFrustum&>(lightFrc->getFrustum());

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
void Is::setState(CommandBufferHandle& cmdBuff)
{
	/*Bool drawToDefaultFbo = !m_r->getPps().getEnabled() 
		&& !m_r->getDbg().getEnabled() 
		&& !m_r->getIsOffscreen()
		&& m_r->getRenderingQuality() == 1.0;*/
	Bool drawToDefaultFbo = false;

	if(drawToDefaultFbo)
	{
		m_r->getDefaultFramebuffer().bind(cmdBuff, true);
		cmdBuff.setViewport(0, 0, 
			m_r->getDefaultFramebufferWidth(), 
			m_r->getDefaultFramebufferHeight());
	}
	else
	{
		m_fb.bind(cmdBuff, true);

		cmdBuff.setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	}
}

//==============================================================================
Error Is::run(CommandBufferHandle& cmdBuff)
{
	// Do the light pass including the shadow passes
	return lightPass(cmdBuff);
}

//==============================================================================
Error Is::updateCommonBlock(CommandBufferHandle& cmdBuff)
{
	SceneGraph& scene = m_r->getSceneGraph();
	shader::CommonUniforms blk;

	// Start writing
	blk.m_projectionParams = m_r->getProjectionParameters();
	blk.m_sceneAmbientColor = scene.getAmbientColor();

	Vec3 groundLightDir;
	if(m_groundLightEnabled)
	{
		const Mat4& viewMat = 
			m_cam->getComponent<FrustumComponent>().getViewMatrix();
		blk.m_groundLightDir = Vec4(-viewMat.getColumn(1).xyz(), 1.0);
	}

	m_commonBuffer.write(cmdBuff, &blk, sizeof(blk), 0, 0, sizeof(blk));

	return ErrorCode::NONE;
}

//==============================================================================
PtrSize Is::calcLightsBufferSize() const
{
	U32 buffAlignment = 
		getGrManager().getBufferOffsetAlignment(GL_SHADER_STORAGE_BUFFER);
	PtrSize size;

	size = getAlignedRoundUp(
		buffAlignment,
		m_maxPointLights * sizeof(shader::PointLight));

	size += getAlignedRoundUp(
		buffAlignment,
		m_maxSpotLights * sizeof(shader::SpotLight));

	size += getAlignedRoundUp(
		buffAlignment,
		m_maxSpotTexLights * sizeof(shader::SpotLight));

	return size;
}

} // end namespace anki
