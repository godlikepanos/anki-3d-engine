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
	Plane planesX[Is::TILES_X_COUNT - 1];
	Plane planesY[Is::TILES_Y_COUNT - 1];
	Plane planesNear[Is::TILES_Y_COUNT][Is::TILES_X_COUNT];
	Plane planesFar[Is::TILES_Y_COUNT][Is::TILES_X_COUNT];
};

struct Tile
{
	U32 lightsCount[4]; 
	U32 pointLightIndices[Is::MAX_POINT_LIGHTS_PER_TILE];
	U32 spotLightIndices[Is::MAX_SPOT_LIGHTS_PER_TILE];
	U32 spotTexLightndices[Is::MAX_SPOT_TEX_LIGHTS_PER_TILE];
};

struct Tiles
{
	Tile tiles[Is::TILES_Y_COUNT][Is::TILES_X_COUNT];
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
	Vec4 extendPoints[4]; 
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

	SceneVector<Light*>::iterator lightsBegin; // XXX Make them const and ShaderNode
	SceneVector<Light*>::iterator lightsEnd;

	std::atomic<U32>* pointLightsCount = nullptr;
	std::atomic<U32>* spotLightsCount = nullptr;
	std::atomic<U32>* spotTexLightsCount = nullptr;
		
	std::atomic<U32> 
		(*tilePointLightsCount)[TILES_Y_COUNT][TILES_X_COUNT];
	std::atomic<U32> 
		(*tileSpotLightsCount)[TILES_Y_COUNT][TILES_X_COUNT];
	std::atomic<U32> 
		(*tileSpotTexLightsCount)[TILES_Y_COUNT][TILES_X_COUNT];

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
			Light* light = lightsBegin + i;

			switch(light->getLightType())
			{
			case Light::LT_POINT:
				doLight(*static_cast<PointLight*>(light));
				break;
			case Light::LT_SPOT:
				doLight(*static_cast<SpotLight*>(light));
				break;
			}
		}
	}

	/// Copy CPU light to GPU buffer
	U doLight(PointLight& light)
	{
		// Get GPU light
		U32 pos = pointLightsCount->fetch_add(1);
		ANKI_ASSERT(pos < MAX_POINT_LIGHTS);

		shader::PointLight& slight = pointLights[pos];

		const Camera* cam = is->cam;
		ANKI_ASSERT(cam);
	
		Vec3 pos = light.getWorldTransform().getOrigin().getTransformed(
			cam->getViewMatrix());

		slight.posRadius = Vec4(pos, light.getRadius());
		slight.diffuseColorShadowmapId = light.getDiffuseColor();
		slight.specularColorTexId = light.getSpecularColor();

		return pos;
	}

	/// Copy CPU spot light to GPU buffer
	U doLight(SpotLight& light)
	{
		Bool isTexLight = light.getShadowEnabled();

		U32 pos;
		shader::SpotLight* baseslight = nullptr;
		if(isTexLight)
		{
			// Spot tex light

			pos = spotTexLightsCount->fetch_add(1);
			ANKI_ASSERT(pos < MAX_SPOT_TEX_LIGHTS);

			shader::SpotTexLight& slight = spotTexLights[pos];
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

			// Write tex IDs
			slight.diffuseColorShadowmapId.w() = pos;
		}
		else
		{
			// Spot light without texture

			pos = spotLightsCount->fetch_add(1);
			ANKI_ASSERT(pos < MAX_SPOT_LIGHTS);

			shader::SpotLight& slight = spotLights[pos];
			baseslight = &slight;
		}

		// Write common stuff
		ANKI_ASSERT(baseslight);

		// Pos & dist
		Vec3 pos = light.getWorldTransform().getOrigin().getTransformed(
				cam->getViewMatrix());
		baseslight->posRadius = Vec4(pos, light.getDistance());

		// Diff color
		baseslight->diffuseColorShadowmapId = Vec4(
			light.getDiffuseColor().xyz(), 
			slight.diffuseColorShadowmapId.w());

		// Spec color
		slight.specularColorTexId = light.getSpecularColor();

		// Light dir
		Vec3 lightDir = -light.getWorldTransform().getRotation().getZAxis();
		lightDir = cam->getViewMatrix().getRotationPart() * lightDir;
		baseslight->lightDir = Vec4(lightDir, 0.0);
		
		// Angles
		slight.outerCosInnerCos = Vec4(light.getOuterAngleCos(),
			light.getInnerAngleCos(), 1.0, 1.0);

		// extend points
		const PerspectiveFrustum& frustum = light.getFrustum();

		for(U i = 0; i < 4; i++)
		{
			Vec3 dir = light.getWorldTransform().getOrigin() 
				+ frustum.getDirections()[i];
			dir.transform(cam->getViewMatrix());
			baseslight->extendPoints[i] = Vec4(dir, 1.0);
		}

		return pos;
	}

	// Bin point light
	void binLight(const PointLight& light, U pos)
	{
		// Do the tests
		Tiler::Bitset bitset;
		tiler->test(light->getSpatialCollisonShape(), true, &bitset);

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
		tiler->test(light->getSpatialCollisonShape(), true, &bitset);

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
				tiles->tiles[y][x].spotTexLightIndices[tilePos] = pos;
			}
			else
			{
				U tilePos = (*tileSpotLightsCount)[y][x].fetch_add(1);
				tiles->tiles[y][x].spotLightIndices[tilePos] = pos;
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
		"#define TILES_X_COUNT " + std::to_string(Tiler::TILES_X_COUNT) + "\n"
		"#define TILES_Y_COUNT " + std::to_string(Tiler::TILES_Y_COUNT) + "\n"
		"#define RENDERER_WIDTH " + std::to_string(r->getWidth()) + "\n"
		"#define RENDERER_HEIGHT " + std::to_string(r->getHeight()) + "\n"
		"#define MAX_LIGHTS_PER_TILE " + std::to_string(MAX_LIGHTS_PER_TILE)
		+ "\n"
		"#define MAX_POINT_LIGHTS " + std::to_string(MAX_POINT_LIGHTS) + "\n"
		"#define MAX_SPOT_LIGHTS " + std::to_string(MAX_SPOT_LIGHTS) + "\n"
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

	tilerProg.load(ShaderProgramResource::createSrcCodeToCache(
		"shaders/IsUpdateTiles.glsl", pps.c_str()).c_str());

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
	lightsUbo.create()

	pointLightsUbo.create(LIGHTS_UBO_SIZE, nullptr);

	// tiles UBO
	tilesUbo.create(sizeof(shader::Tiles), nullptr);

	// tilegrid
	tilegridBuffer.create(sizeof(shader::Tilegrid), GL_SHADER_STORAGE_BUFFER,
		nullptr, GL_STATIC_DRAW);

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
	visiblePointLightsCount = MAX_POINT_LIGHTS % visiblePointLightsCount;
	visibleSpotLightsCount = MAX_SPOT_LIGHTS % visibleSpotLightsCount;
	visibleSpotTexLightsCount = MAX_SPOT_TEX_LIGHTS % visibleSpotTexLightsCount;

	//
	// Do shadows pass
	//
	Array<U32, Sm::MAX_SHADOW_CASTERS> shadowmapLayers;
	sm.run(&shadowCasters[0], spotsShadowCount, shadowmapLayers);

	//
	// Prepare state
	//
	fbo.bind();
	r->clearAfterBindingFbo(
		GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	GlStateSingleton::get().setViewport(
		0, 0, r->getWidth(), r->getHeight());
	GlStateSingleton::get().disable(GL_DEPTH_TEST);
	GlStateSingleton::get().disable(GL_BLEND);

	//
	// Write the lights UBO
	//

	U8 clientBuffer[LIGHTS_UBO_SIZE];

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

	shader::Tiles tilesClient;

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

		job.pointLigths = 
			(shader::PointLight*)(&clientBuffer[0] + pointLightsOffset);
		job.spotLigths = 
			(shader::SpotLight*)(&clientBuffer[0] + spotLightsOffset);
		job.spotTexLigths = 
			(shader::SpotTexLight*)(&clientBuffer[0] + spotTexLightsOffset);

		job.tiles = &tilesClient;

		// XXX add more

		job.pointLightsCount = &pointLightsAtomicCount;
		job.spotLightsCount = &spotLightsAtomicCount;
		job.spotTexLightsCount = &spotTexLightsAtomicCount;

		job.tilePointLightsCount = &tilePointLightsCount;
		job.tileSpotLightsCount = &tileSpotLightsCount;
		job.tileSpotTexLightsCount = &tileSpotTexLightsCount;

		job.tiler = r->getTiler();
		job.is = this;
	}

	// Sync
	threadPool.waitForAllJobsToFinish();

	// Write BO
	lightsUbo.write(&clientBuffer[0], spotTexLightsOffset + spotTexLightsSize);

	//
	// Update the tiles UBO
	//

	shader::Tiles clientBuffer;

	WriteTilesUboJob tjobs[ThreadPool::MAX_THREADS];
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		tjobs[i].visiblePointLights = &visiblePointLights[0];
		tjobs[i].visiblePointLightsCount = visiblePointLightsCount;

		tjobs[i].visibleSpotLights = &visibleSpotLights[0];
		tjobs[i].visibleSpotLightsCount = visibleSpotLightsCount;

		tjobs[i].shaderTiles = &clientBuffer.tiles[0];
		tjobs[i].is = this;

		threadPool.assignNewJob(i, &tjobs[i]);
	}

	threadPool.waitForAllJobsToFinish();

	tilesUbo.write(&clientBuffer);

	// XXX
	/*{
		tilesUbo.bind(GL_SHADER_STORAGE_BUFFER);
		tilesUbo.setBinding(0);

		tilerProg->bind();

		glDispatchCompute(Tiler::TILES_X_COUNT, Tiler::TILES_Y_COUNT, 1);

		tilesUbo.bind(GL_UNIFORM_BUFFER);
	}*/

	//
	// Setup shader and draw
	//
	
	// shader prog
	lightPassProg->bind();

	lightPassProg->findUniformVariable("limitsOfNearPlane").set(
		Vec4(r->getLimitsOfNearPlane(), r->getLimitsOfNearPlane2()));

	commonUbo.setBinding(COMMON_UNIFORMS_BLOCK_BINDING);
	pointLightsUbo.setBinding(POINT_LIGHTS_BLOCK_BINDING);
	spotLightsUbo.setBinding(SPOT_LIGHTS_BLOCK_BINDING);
	tilesUbo.setBinding(TILES_BLOCK_BINDING);

	lightPassProg->findUniformVariable("msFai0").set(r->getMs().getFai0());
	lightPassProg->findUniformVariable("msDepthFai").set(
		r->getMs().getDepthFai());
	lightPassProg->findUniformVariable("shadowMapArr").set(sm.sm2DArrayTex);

	quadVao.bind();
	glDrawElementsInstanced(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_SHORT, 0,
		Tiler::TILES_Y_COUNT * Tiler::TILES_X_COUNT);
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
