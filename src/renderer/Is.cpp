#include "anki/renderer/Is.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Light.h"
#include "anki/core/ThreadPool.h"

namespace anki {

//==============================================================================
static const U MAX_POINT_LIGHTS_PER_TILE = 
	ANKI_RENDERER_MAX_POINT_LIGHTS_PER_TILE;
static const U MAX_SPOT_LIGHTS_PER_TILE = 
	ANKI_RENDERER_MAX_SPOT_LIGHTS_PER_TILE;
static const U MAX_SPOT_TEX_LIGHTS_PER_TILE = 
	ANKI_RENDERER_MAX_SPOT_TEX_LIGHTS_PER_TILE;

static const U MAX_POINT_LIGHTS = ANKI_RENDERER_MAX_POINT_LIGHTS;
static const U MAX_SPOT_LIGHTS = ANKI_RENDERER_MAX_SPOT_LIGHTS;
static const U MAX_SPOT_TEX_LIGHTS = ANKI_RENDERER_MAX_SPOT_TEX_LIGHTS;
static const U MAX_LIGHTS = 
	MAX_POINT_LIGHTS + MAX_SPOT_LIGHTS + MAX_SPOT_TEX_LIGHTS;

static const U TILES_X_COUNT = ANKI_RENDERER_TILES_X_COUNT;
static const U TILES_Y_COUNT = ANKI_RENDERER_TILES_Y_COUNT;
static const U TILES_COUNT = TILES_X_COUNT * TILES_Y_COUNT;

//==============================================================================
static Bool groundVectorsEqual(const Vec3& prev, const Vec3& crnt)
{
	Bool out = true;
	for(U i = 0; i < 3; i++)
	{
		Bool subout = fabs(prev[i] - crnt[i]) < (getEpsilon<F32>() * 10.0);
		out = out && subout;
	}

	return out;
}

//==============================================================================

// Shader structs and block representations. All positions and directions in
// viewspace
// For documentation see the shaders

namespace shader {

struct Plane
{
	Vec3 normal;
	F32 offset;
};

struct Tilegrid
{
	Plane planesX[TILES_X_COUNT - 1];
	Plane planesY[TILES_Y_COUNT - 1];
	Plane planesNear[TILES_Y_COUNT][TILES_X_COUNT];
	Plane planesFar[TILES_Y_COUNT][TILES_X_COUNT];
};

struct Tile
{
	Array<U32, 4> lightsCount; 
	Array<U32, MAX_POINT_LIGHTS_PER_TILE> pointLightIndices;
	Array<U32, MAX_SPOT_LIGHTS_PER_TILE> spotLightIndices;
	Array<U32, MAX_SPOT_TEX_LIGHTS_PER_TILE> spotTexLightIndices;
};

struct Tiles
{
	Tile tiles[TILES_Y_COUNT][TILES_X_COUNT];
};

struct Light
{
	Vec4 posRadius;
	Vec4 diffuseColorShadowmapId;
	Vec4 specularColorTexId;
};

struct PointLight: Light
{};

struct SpotLight: Light
{
	Vec4 lightDir;
	Vec4 outerCosInnerCos;
	Array<Vec4, 4> extendPoints; 
};

struct SpotTexLight: SpotLight
{
	Mat4 texProjectionMat; ///< Texture projection matrix
};

struct CommonUniforms
{
	Vec4 planes;
	Vec4 sceneAmbientColor;
	Vec4 groundLightDir;
};

} // end namespace shader

static const PtrSize LIGHTS_UBO_SIZE = 
	MAX_POINT_LIGHTS * sizeof(shader::PointLight)
	+ MAX_SPOT_LIGHTS * sizeof(shader::SpotLight)
	+ MAX_SPOT_TEX_LIGHTS * sizeof(shader::SpotTexLight);

//==============================================================================

// Threading jobs

/// Write the lights to a client buffer
struct WriteLightsJob: ThreadJob
{
	shader::PointLight* pointLights = nullptr;
	shader::SpotLight* spotLights = nullptr;
	shader::SpotTexLight* spotTexLights = nullptr;

	shader::Tiles* tiles = nullptr;

	VisibilityTestResults::Container::const_iterator lightsBegin;
	VisibilityTestResults::Container::const_iterator lightsEnd;

	std::atomic<U32>* pointLightsCount = nullptr;
	std::atomic<U32>* spotLightsCount = nullptr;
	std::atomic<U32>* spotTexLightsCount = nullptr;
		
	std::atomic<U32> 
		(*tilePointLightsCount)[TILES_Y_COUNT][TILES_X_COUNT] = nullptr;
	std::atomic<U32> 
		(*tileSpotLightsCount)[TILES_Y_COUNT][TILES_X_COUNT] = nullptr;
	std::atomic<U32> 
		(*tileSpotTexLightsCount)[TILES_Y_COUNT][TILES_X_COUNT] = nullptr;

	Tiler* tiler = nullptr;
	Is* is = nullptr;

	void operator()(U threadId, U threadsCount)
	{
		U ligthsCount = lightsEnd - lightsBegin;

		// Count job bounds
		U64 start, end;
		choseStartEnd(threadId, threadsCount, ligthsCount, start, end);

		// Run all lights
		for(U64 i = start; i < end; i++)
		{
			const SceneNode* snode = *(lightsBegin + i);
			const Light* light = snode->getLight();
			ANKI_ASSERT(light);

			switch(light->getLightType())
			{
			case Light::LT_POINT:
				{
					const PointLight& l = 
						*static_cast<const PointLight*>(light);
					I pos = doLight(l);
					if(pos != -1)
					{
						binLight(l, pos);
					}
				}
				break;
			case Light::LT_SPOT:
				{
					const SpotLight& l = 
						*static_cast<const SpotLight*>(light);
					I pos = doLight(l);
					if(pos != -1)
					{
						binLight(l, pos);
					}
				}
				break;
			default:
				ANKI_ASSERT(0);
				break;
			}
		}
	}

	/// Copy CPU light to GPU buffer
	I doLight(const PointLight& light)
	{
		// Get GPU light
		I i = pointLightsCount->fetch_add(1);
		if(i >= (I)MAX_POINT_LIGHTS)
		{
			return -1;
		}

		shader::PointLight& slight = pointLights[i];

		const Camera* cam = is->cam;
		ANKI_ASSERT(cam);
	
		Vec3 pos = light.getWorldTransform().getOrigin().getTransformed(
			cam->getViewMatrix());

		slight.posRadius = Vec4(pos, light.getRadius());
		slight.diffuseColorShadowmapId = light.getDiffuseColor();
		slight.specularColorTexId = light.getSpecularColor();

		return i;
	}

	/// Copy CPU spot light to GPU buffer
	I doLight(const SpotLight& light)
	{
		const Camera* cam = is->cam;
		Bool isTexLight = light.getShadowEnabled();
		I i;
		shader::SpotLight* baseslight = nullptr;

		if(isTexLight)
		{
			// Spot tex light

			i = spotTexLightsCount->fetch_add(1);
			if(i >= (I)MAX_SPOT_TEX_LIGHTS)
			{
				return -1;
			}

			shader::SpotTexLight& slight = spotTexLights[i];
			baseslight = &slight;

			// Write matrix
			static const Mat4 biasMat4(
				0.5, 0.0, 0.0, 0.5, 
				0.0, 0.5, 0.0, 0.5, 
				0.0, 0.0, 0.5, 0.5, 
				0.0, 0.0, 0.0, 1.0);
			// bias * proj_l * view_l * world_c
			slight.texProjectionMat = biasMat4 * light.getProjectionMatrix() *
				Mat4::combineTransformations(light.getViewMatrix(),
				Mat4(cam->getWorldTransform()));

			// Transpose because of driver bug
			slight.texProjectionMat.transpose();
		}
		else
		{
			// Spot light without texture

			i = spotLightsCount->fetch_add(1);
			if(i >= (I)MAX_SPOT_LIGHTS)
			{
				return -1;
			}

			shader::SpotLight& slight = spotLights[i];
			baseslight = &slight;
		}

		// Write common stuff
		ANKI_ASSERT(baseslight);

		// Pos & dist
		Vec3 pos = light.getWorldTransform().getOrigin().getTransformed(
				cam->getViewMatrix());
		baseslight->posRadius = Vec4(pos, light.getDistance());

		// Diff color. don't set the shadowmap ID now
		baseslight->diffuseColorShadowmapId = light.getDiffuseColor();

		// Spec color
		baseslight->specularColorTexId = light.getSpecularColor();

		// Light dir
		Vec3 lightDir = -light.getWorldTransform().getRotation().getZAxis();
		lightDir = cam->getViewMatrix().getRotationPart() * lightDir;
		baseslight->lightDir = Vec4(lightDir, 0.0);
		
		// Angles
		baseslight->outerCosInnerCos = Vec4(
			light.getOuterAngleCos(),
			light.getInnerAngleCos(), 
			1.0, 
			1.0);

		// extend points
		const PerspectiveFrustum& frustum = light.getFrustum();

		for(U i = 0; i < 4; i++)
		{
			Vec3 extendPoint = light.getWorldTransform().getOrigin() 
				+ frustum.getDirections()[i];

			extendPoint.transform(cam->getViewMatrix());
			baseslight->extendPoints[i] = Vec4(extendPoint, 1.0);
		}

		return i;
	}

	// Bin point light
	void binLight(const PointLight& light, U pos)
	{
		// Do the tests
		Tiler::Bitset bitset;
		tiler->test(light.getSpatialCollisionShape(), true, &bitset);

		// Bin to the correct tiles
		for(U t = 0; t < TILES_COUNT; t++)
		{
			// If not in tile bye
			if(!bitset.test(t))
			{
				continue;
			}

			U x = t % TILES_X_COUNT;
			U y = t / TILES_X_COUNT;

			U tilePos = (*tilePointLightsCount)[y][x].fetch_add(1);

			if(pos < MAX_POINT_LIGHTS_PER_TILE)
			{
				tiles->tiles[y][x].pointLightIndices[tilePos] = pos;
			}
		}
	}

	// Bin spot light
	void binLight(const SpotLight& light, U pos)
	{
		// Do the tests
		Tiler::Bitset bitset;
		tiler->test(light.getSpatialCollisionShape(), true, &bitset);

		// Bin to the correct tiles
		for(U t = 0; t < TILES_COUNT; t++)
		{
			// If not in tile bye
			if(!bitset.test(t))
			{
				continue;
			}

			U x = t % TILES_X_COUNT;
			U y = t / TILES_X_COUNT;

			if(light.getShadowEnabled())
			{
				U tilePos = (*tileSpotTexLightsCount)[y][x].fetch_add(1);

				if(tilePos < MAX_SPOT_TEX_LIGHTS_PER_TILE)
				{
					tiles->tiles[y][x].spotTexLightIndices[tilePos] = pos;
				}
			}
			else
			{
				U tilePos = (*tileSpotLightsCount)[y][x].fetch_add(1);

				if(tilePos < MAX_SPOT_LIGHTS_PER_TILE)
				{
					tiles->tiles[y][x].spotLightIndices[tilePos] = pos;
				}
			}
		}
	}
};

//==============================================================================
Is::Is(Renderer* r_)
	: RenderingPass(r_), sm(r_)
{}

//==============================================================================
Is::~Is()
{}

//==============================================================================
void Is::init(const RendererInitializer& initializer)
{
	try
	{
		initInternal(initializer);
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to init IS") << e;
	}
}

//==============================================================================
void Is::initInternal(const RendererInitializer& initializer)
{
	groundLightEnabled = initializer.is.groundLightEnabled;

	//
	// Init the passes
	//
	sm.init(initializer);

	//
	// Load the programs
	//
	std::string pps =
		"#define TILES_X_COUNT " + std::to_string(TILES_X_COUNT) + "\n"
		"#define TILES_Y_COUNT " + std::to_string(TILES_Y_COUNT) + "\n"
		"#define TILES_COUNT " + std::to_string(TILES_COUNT) + "\n"
		"#define RENDERER_WIDTH " + std::to_string(r->getWidth()) + "\n"
		"#define RENDERER_HEIGHT " + std::to_string(r->getHeight()) + "\n"
		"#define MAX_POINT_LIGHTS_PER_TILE " 
		+ std::to_string(MAX_POINT_LIGHTS_PER_TILE) + "\n"
		"#define MAX_SPOT_LIGHTS_PER_TILE " 
		+ std::to_string(MAX_SPOT_LIGHTS_PER_TILE) + "\n"
		"#define MAX_SPOT_TEX_LIGHTS_PER_TILE " 
		+ std::to_string(MAX_SPOT_TEX_LIGHTS_PER_TILE) + "\n"
		"#define MAX_POINT_LIGHTS " + std::to_string(MAX_POINT_LIGHTS) + "\n"
		"#define MAX_SPOT_LIGHTS " + std::to_string(MAX_SPOT_LIGHTS) + "\n"
		"#define MAX_SPOT_TEX_LIGHTS " 
		+ std::to_string(MAX_SPOT_TEX_LIGHTS) + "\n"
		"#define GROUND_LIGHT " + std::to_string(groundLightEnabled) + "\n";

	if(sm.getPcfEnabled())
	{
		pps += "#define PCF 1\n";
	}
	else
	{
		pps += "#define PCF 0\n";
	}

	// point light
	lightPassProg.load(ShaderProgramResource::createSrcCodeToCache(
		"shaders/IsLp.glsl", pps.c_str()).c_str());

	//
	// Create FBOs
	//

	// IS FBO
	Renderer::createFai(r->getWidth(), r->getHeight(), GL_RGB8,
		GL_RGB, GL_UNSIGNED_BYTE, fai);
	fbo.create();
	fbo.setColorAttachments({&fai});
	fbo.setOtherAttachment(GL_DEPTH_ATTACHMENT, r->getMs().getDepthFai());

	if(!fbo.isComplete())
	{
		throw ANKI_EXCEPTION("Fbo not complete");
	}

	//
	// Init the quad
	//
	static const F32 quadVertCoords[][2] = {{1.0, 1.0}, {0.0, 1.0},
		{0.0, 0.0}, {1.0, 0.0}};
	quadPositionsVbo.create(GL_ARRAY_BUFFER, sizeof(quadVertCoords),
		quadVertCoords, GL_STATIC_DRAW);

	static const U16 quadVertIndeces[2][3] =
		{{0, 1, 3}, {1, 2, 3}}; // 2 triangles
	quadVertIndecesVbo.create(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadVertIndeces),
		quadVertIndeces, GL_STATIC_DRAW);

	quadVao.create();
	quadVao.attachArrayBufferVbo(
		&quadPositionsVbo, 0, 2, GL_FLOAT, false, 0, 0);
	quadVao.attachElementArrayBufferVbo(&quadVertIndecesVbo);

	//
	// Create UBOs
	//

	// Common UBO
	commonUbo.create(sizeof(shader::CommonUniforms), nullptr);

	// lights UBO
	lightsUbo.create(LIGHTS_UBO_SIZE, nullptr);

	// tiles UBO
	tilesUbo.create(sizeof(shader::Tiles), nullptr);

	// tilegrid
	tilegridBuffer.create(GL_SHADER_STORAGE_BUFFER, sizeof(shader::Tilegrid),
		nullptr, GL_STATIC_DRAW);

	ANKI_LOGI("Creating BOs: lights: " << LIGHTS_UBO_SIZE << "B tiles: "
		<< sizeof(shader::Tiles) << "B");

	// Sanity checks
	const ShaderProgramUniformBlock* ublock;

	ublock = &lightPassProg->findUniformBlock("commonBlock");
	ublock->setBinding(COMMON_UNIFORMS_BLOCK_BINDING);
	if(ublock->getSize() != sizeof(shader::CommonUniforms)
		|| ublock->getBinding() != COMMON_UNIFORMS_BLOCK_BINDING)
	{
		throw ANKI_EXCEPTION("Problem with the commonBlock");
	}

	ublock = &lightPassProg->findUniformBlock("pointLightsBlock");
	ublock->setBinding(POINT_LIGHTS_BLOCK_BINDING);
	if(ublock->getSize() != sizeof(shader::PointLight) * MAX_POINT_LIGHTS
		|| ublock->getBinding() != POINT_LIGHTS_BLOCK_BINDING)
	{
		throw ANKI_EXCEPTION("Problem with the pointLightsBlock");
	}

	ublock = &lightPassProg->findUniformBlock("spotLightsBlock");
	ublock->setBinding(SPOT_LIGHTS_BLOCK_BINDING);
	if(ublock->getSize() != sizeof(shader::SpotLight) * MAX_SPOT_LIGHTS
		|| ublock->getBinding() != SPOT_LIGHTS_BLOCK_BINDING)
	{
		throw ANKI_EXCEPTION("Problem with the spotLightsBlock");
	}

	ublock = &lightPassProg->findUniformBlock("spotTexLightsBlock");
	ublock->setBinding(SPOT_TEX_LIGHTS_BLOCK_BINDING);
	if(ublock->getSize() != sizeof(shader::SpotTexLight) * MAX_SPOT_TEX_LIGHTS
		|| ublock->getBinding() != SPOT_TEX_LIGHTS_BLOCK_BINDING)
	{
		throw ANKI_EXCEPTION("Problem with the spotTexLightsBlock");
	}

	ublock = &lightPassProg->findUniformBlock("tilesBlock");
	ublock->setBinding(TILES_BLOCK_BINDING);
	if(ublock->getSize() != sizeof(shader::Tiles)
		|| ublock->getBinding() != TILES_BLOCK_BINDING)
	{
		throw ANKI_EXCEPTION("Problem with the tilesBlock");
	}
}

//==============================================================================
void Is::lightPass()
{
	ThreadPool& threadPool = ThreadPoolSingleton::get();
	VisibilityTestResults& vi = 
		*cam->getFrustumable()->getVisibilityTestResults();

	//
	// Quickly get the lights
	//
	U visiblePointLightsCount = 0;
	U visibleSpotLightsCount = 0;
	U visibleSpotTexLightsCount = 0;
	Array<Light*, Sm::MAX_SHADOW_CASTERS> shadowCasters;

	for(auto it = vi.getLightsBegin(); it != vi.getLightsEnd(); ++it)
	{
		Light* light = (*it)->getLight();
		ANKI_ASSERT(light);
		switch(light->getLightType())
		{
		case Light::LT_POINT:
			// Use % to avoid overflows
			++visiblePointLightsCount;
			break;
		case Light::LT_SPOT:
			{
				SpotLight* slight = static_cast<SpotLight*>(light);
				
				if(slight->getShadowEnabled())
				{
					shadowCasters[visibleSpotTexLightsCount++] = slight;
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

	// Sanitize the counts
	visiblePointLightsCount = (visiblePointLightsCount > 0)
		? visiblePointLightsCount % MAX_POINT_LIGHTS : visiblePointLightsCount;
	visibleSpotLightsCount = (visibleSpotLightsCount > 0) 
		? visibleSpotLightsCount % MAX_SPOT_LIGHTS : visibleSpotLightsCount;
	visibleSpotTexLightsCount = (visibleSpotTexLightsCount > 0) 
		? visibleSpotTexLightsCount % MAX_SPOT_TEX_LIGHTS 
		: visibleSpotTexLightsCount;

	//
	// Do shadows pass
	//
	Array<U32, Sm::MAX_SHADOW_CASTERS> shadowmapLayers;
	sm.run(&shadowCasters[0], visibleSpotTexLightsCount, shadowmapLayers);

	//
	// Write the ligths and tiles UBOs
	//

	// Get the offsets and sizes of each uniform block
	PtrSize pointLightsOffset = 0;
	PtrSize pointLightsSize = 
		sizeof(shader::PointLight) * visiblePointLightsCount;

	PtrSize spotLightsOffset = pointLightsSize;
	PtrSize spotLightsSize = 
		sizeof(shader::SpotLight) * visibleSpotLightsCount;

	PtrSize spotTexLightsOffset = spotLightsOffset + spotLightsSize;
	PtrSize spotTexLightsSize = 
		sizeof(shader::SpotTexLight) * visibleSpotTexLightsCount;

	ANKI_ASSERT(spotTexLightsOffset + spotTexLightsSize <= LIGHTS_UBO_SIZE);

	// Fire the super jobs
	WriteLightsJob jobs[ThreadPool::MAX_THREADS];

	U8 clientBuffer[LIGHTS_UBO_SIZE];
	shader::Tiles clientTiles;

	std::atomic<U32> pointLightsAtomicCount;
	std::atomic<U32> spotLightsAtomicCount;
	std::atomic<U32> spotTexLightsAtomicCount;

	std::atomic<U32> 
		tilePointLightsCount[TILES_Y_COUNT][TILES_X_COUNT];
	std::atomic<U32> 
		tileSpotLightsCount[TILES_Y_COUNT][TILES_X_COUNT];
	std::atomic<U32> 
		tileSpotTexLightsCount[TILES_Y_COUNT][TILES_X_COUNT];

	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		WriteLightsJob& job = jobs[i];

		job.pointLights = 
			(shader::PointLight*)(&clientBuffer[0] + pointLightsOffset);
		job.spotLights = 
			(shader::SpotLight*)(&clientBuffer[0] + spotLightsOffset);
		job.spotTexLights = 
			(shader::SpotTexLight*)(&clientBuffer[0] + spotTexLightsOffset);

		job.tiles = &clientTiles;

		job.lightsBegin = vi.getLightsBegin();
		job.lightsEnd = vi.getLightsEnd();

		job.pointLightsCount = &pointLightsAtomicCount;
		job.spotLightsCount = &spotLightsAtomicCount;
		job.spotTexLightsCount = &spotTexLightsAtomicCount;

		job.tilePointLightsCount = &tilePointLightsCount;
		job.tileSpotLightsCount = &tileSpotLightsCount;
		job.tileSpotTexLightsCount = &tileSpotTexLightsCount;

		job.tiler = &r->getTiler();
		job.is = this;

		threadPool.assignNewJob(i, &job);
	}

	// In the meantime set the state
	setState();

	// Sync
	threadPool.waitForAllJobsToFinish();

	// Write the shadomap IDs
	for(U i = 0; i < visibleSpotTexLightsCount; i++)
	{
		jobs[0].spotTexLights[i].diffuseColorShadowmapId.w() = 
			(F32)shadowmapLayers[i];
	}

	// Write the light count for each tile
	for(U y = 0; y < TILES_Y_COUNT; y++)
	{
		for(U x = 0; x < TILES_X_COUNT; x++)
		{
			clientTiles.tiles[y][x].lightsCount[0] = 
				tilePointLightsCount[y][x].load();

			clientTiles.tiles[y][x].lightsCount[2] = 
				tileSpotLightsCount[y][x].load();

			clientTiles.tiles[y][x].lightsCount[3] = 
				tileSpotTexLightsCount[y][x].load();
		}
	}

	// Write BOs
	lightsUbo.write(
		&clientBuffer[0], 0, spotTexLightsOffset + spotTexLightsSize);
	tilesUbo.write(&clientTiles);

	//
	// Setup shader and draw
	//

	// shader prog
	lightPassProg->bind();

	lightPassProg->findUniformVariable("limitsOfNearPlane").set(
		Vec4(r->getLimitsOfNearPlane(), r->getLimitsOfNearPlane2()));

	commonUbo.setBinding(COMMON_UNIFORMS_BLOCK_BINDING);

	if(pointLightsSize > 0)
	{
		lightsUbo.setBindingRange(POINT_LIGHTS_BLOCK_BINDING, pointLightsOffset,
			pointLightsSize);
	}
	if(spotLightsSize > 0)
	{
		lightsUbo.setBindingRange(SPOT_LIGHTS_BLOCK_BINDING, spotLightsOffset,
			spotLightsSize);
	}
	if(spotTexLightsSize > 0)
	{
		lightsUbo.setBindingRange(SPOT_TEX_LIGHTS_BLOCK_BINDING, 
			spotTexLightsOffset, spotTexLightsSize);
	}
	tilesUbo.setBinding(TILES_BLOCK_BINDING);

	lightPassProg->findUniformVariable("msFai0").set(r->getMs().getFai0());
	lightPassProg->findUniformVariable("msDepthFai").set(
		r->getMs().getDepthFai());
	lightPassProg->findUniformVariable("shadowMapArr").set(sm.sm2DArrayTex);

	quadVao.bind();
	glDrawElementsInstanced(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_SHORT, 0,
		TILES_COUNT);
}

//==============================================================================
void Is::setState()
{
	fbo.bind();
	r->clearAfterBindingFbo(
		GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	GlStateSingleton::get().setViewport(
		0, 0, r->getWidth(), r->getHeight());
	GlStateSingleton::get().disable(GL_DEPTH_TEST);
	GlStateSingleton::get().disable(GL_BLEND);
}

//==============================================================================
void Is::run()
{
	SceneGraph& scene = r->getSceneGraph();
	cam = &scene.getActiveCamera();

	GlStateSingleton::get().disable(GL_BLEND);

	// Ground light direction
	Vec3 groundLightDir;
	if(groundLightEnabled)
	{
		groundLightDir = -cam->getViewMatrix().getColumn(1).xyz();
	}

	// Write common block
	if(commonUboUpdateTimestamp < r->getPlanesUpdateTimestamp()
		|| commonUboUpdateTimestamp < scene.getAmbientColorUpdateTimestamp()
		|| commonUboUpdateTimestamp == 1
		|| (groundLightEnabled
			&& !groundVectorsEqual(groundLightDir, prevGroundLightDir)))
	{
		shader::CommonUniforms blk;
		blk.planes = Vec4(r->getPlanes().x(), r->getPlanes().y(), 0.0, 0.0);
		blk.sceneAmbientColor = Vec4(scene.getAmbientColor(), 0.0);

		if(groundLightEnabled)
		{
			blk.groundLightDir = Vec4(groundLightDir, 1.0);
			prevGroundLightDir = groundLightDir;
		}

		commonUbo.write(&blk);

		commonUboUpdateTimestamp = Timestamp::getTimestamp();
	}

	// Do the light pass including the shadow passes
	lightPass();
}

} // end namespace anki
