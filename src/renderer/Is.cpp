#include "anki/renderer/Is.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Light.h"
#include "anki/core/ThreadPool.h"
#include <bitset> // XXX remove it

namespace anki {

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

// Shader structs and block representations

struct ShaderLight
{
	Vec4 posAndRadius; ///< xyz: Light pos in eye space. w: The radius
	Vec4 diffuseColorShadowmapId;
	Vec4 specularColor;
};

struct ShaderPointLight: ShaderLight
{};

struct ShaderSpotLight: ShaderLight
{
	Vec4 lightDirection; ///< xyz: Dir vector
	Vec4 outerCosInnerCos; ///< x: outer angle cos, y: inner
	Mat4 texProjectionMat;
};

struct ShaderPointLights
{
	ShaderPointLight lights[Is::MAX_POINT_LIGHTS];
};

struct ShaderSpotLights
{
	ShaderSpotLight lights[Is::MAX_SPOT_LIGHTS];
};

struct ShaderTile
{
	U32 lightsCount[3]; ///< 0: Point lights number, 1: Spot lights number
	U32 padding[1];
	// When change this change the writeTilesUbo as well
	Array<U32, Is::MAX_LIGHTS_PER_TILE> lightIndices; 
};

struct ShaderTiles
{
	Array<ShaderTile, Tiler::TILES_X_COUNT * Tiler::TILES_Y_COUNT> tiles;
};

struct ShaderCommonUniforms
{
	Vec4 planes;
	Vec4 sceneAmbientColor;
	Vec4 groundLightDir;
};

//==============================================================================

// Threading jobs

/// Job to update point lights
struct WritePointLightsUbo: ThreadJob
{
	ShaderPointLight* shaderLights = nullptr; ///< Mapped UBO
	PointLight** visibleLights = nullptr;
	U32 visibleLightsCount = 0;
	Is* is = nullptr;

	void operator()(U threadId, U threadsCount)
	{
		U64 start, end;
		choseStartEnd(threadId, threadsCount, visibleLightsCount, start, end);
		
		const Camera* cam = is->cam;

		for(U64 i = start; i < end; i++)
		{
			ANKI_ASSERT(i < Is::MAX_POINT_LIGHTS);
			ANKI_ASSERT(i < visibleLightsCount);

			ShaderPointLight& pl = shaderLights[i];
			const PointLight& light = *visibleLights[i];

			Vec3 pos = light.getWorldTransform().getOrigin().getTransformed(
				cam->getViewMatrix());

			pl.posAndRadius = Vec4(pos, light.getRadius());
			pl.diffuseColorShadowmapId = light.getDiffuseColor();
			pl.specularColor = light.getSpecularColor();
		}
	}
};

/// Job to update spot lights
struct WriteSpotLightsUbo: ThreadJob
{
	ShaderSpotLight* shaderLights = nullptr; ///< Mapped UBO
	SpotLight** visibleLights = nullptr;
	U32 visibleLightsCount = 0;
	Is* is = nullptr;

	void operator()(U threadId, U threadsCount)
	{
		U64 start, end;
		choseStartEnd(threadId, threadsCount, visibleLightsCount, start, end);

		const Camera* cam = is->cam;

		for(U64 i = start; i < end; i++)
		{
			ANKI_ASSERT(i < Is::MAX_SPOT_LIGHTS);
			ANKI_ASSERT(i < visibleLightsCount);

			ShaderSpotLight& slight = shaderLights[i];
			const SpotLight& light = *visibleLights[i];

			Vec3 pos = light.getWorldTransform().getOrigin().getTransformed(
				cam->getViewMatrix());

			slight.posAndRadius = Vec4(pos, light.getDistance());

			slight.diffuseColorShadowmapId = Vec4(light.getDiffuseColor().xyz(),
				0);

			slight.specularColor = light.getSpecularColor();

			Vec3 lightDir = -light.getWorldTransform().getRotation().getZAxis();
			lightDir = cam->getViewMatrix().getRotationPart() * lightDir;
			slight.lightDirection = Vec4(lightDir, 0.0);
			
			slight.outerCosInnerCos = Vec4(light.getOuterAngleCos(),
				light.getInnerAngleCos(), 1.0, 1.0);

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
	}
};

/// A job to write the tiles UBO
struct WriteTilesUboJob: ThreadJob
{
	PointLight** visiblePointLights = nullptr;
	U32 visiblePointLightsCount = 0;
	
	SpotLight** visibleSpotLights = nullptr;
	U32 visibleSpotLightsCount = 0; ///< Both shadow and not

	ShaderTile* shaderTiles = nullptr; ///< Mapped UBO
	U32 maxLightsPerTile = 0;
	Is* is = nullptr;

	void operator()(U threadId, U threadsCount)
	{
		U64 start, end;
		choseStartEnd(threadId, threadsCount, 
			Tiler::TILES_X_COUNT * Tiler::TILES_Y_COUNT, start, end);

		for(U32 i = start; i < end; i++)
		{
			ShaderTile& stile = shaderTiles[i];

			doTile(i, stile);
		}
	}

	/// Do a tile
	void doTile(U tileId, ShaderTile& stile)
	{
		auto& lightIndices = stile.lightIndices;

		// Point lights
		//
		U pointLightsInTileCount = 0;
		for(U i = 0; i < visiblePointLightsCount; i++)
		{
			const PointLight& light = *visiblePointLights[i];

			if(light.getTilerBitset().test(tileId))
			{
				// Use % to avoid overflows
				const U id = pointLightsInTileCount % Is::MAX_LIGHTS_PER_TILE;
				lightIndices[id] = i;
				++pointLightsInTileCount;
			}
		}

		stile.lightsCount[0] = pointLightsInTileCount;

		// Spot lights
		//

		U spotLightsInTileCount = 0;
		U spotLightsShadowInTileCount = 0;
		for(U i = 0; i < visibleSpotLightsCount; i++)
		{
			const SpotLight& light = *visibleSpotLights[i];

			if(light.getTilerBitset().test(tileId))
			{
				const U id = (pointLightsInTileCount + spotLightsInTileCount 
					+ spotLightsShadowInTileCount) 
					% Is::MAX_LIGHTS_PER_TILE;

				// Use % to avoid overflows
				lightIndices[id] = i;

				if(light.getShadowEnabled())
				{
					++spotLightsShadowInTileCount;
				}
				else
				{
					++spotLightsInTileCount;
				}
			}
		}

		stile.lightsCount[1] = spotLightsInTileCount;
		stile.lightsCount[2] = spotLightsShadowInTileCount;

#if 0
		U totalLightsInTileCount = std::min(
			pointLightsInTileCount + spotLightsInTileCount 
			+ spotLightsShadowInTileCount,
			Is::MAX_LIGHTS_PER_TILE);

		if(pointLightsInTileCount + spotLightsInTileCount > maxLightsPerTile)
		{
			ANKI_LOGW("Too many lights per tile: " << lightsInTileCount);
		}
#endif
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
		"shaders/IsLpGeneric.glsl", pps.c_str()).c_str());

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
	commonUbo.create(sizeof(ShaderCommonUniforms), nullptr);

	// lights UBO
	pointLightsUbo.create(sizeof(ShaderPointLights), nullptr);
	spotLightsUbo.create(sizeof(ShaderSpotLights), nullptr);

	// lightIndices UBO
	tilesUbo.create(sizeof(ShaderTiles), nullptr);

	// Sanity checks
	const ShaderProgramUniformBlock* ublock;

	ublock = &lightPassProg->findUniformBlock("commonBlock");
	ublock->setBinding(COMMON_UNIFORMS_BLOCK_BINDING);
	if(ublock->getSize() != sizeof(ShaderCommonUniforms)
		|| ublock->getBinding() != COMMON_UNIFORMS_BLOCK_BINDING)
	{
		throw ANKI_EXCEPTION("Problem with the commonBlock");
	}

	ublock = &lightPassProg->findUniformBlock("pointLightsBlock");
	ublock->setBinding(POINT_LIGHTS_BLOCK_BINDING);
	if(ublock->getSize() != sizeof(ShaderPointLights)
		|| ublock->getBinding() != POINT_LIGHTS_BLOCK_BINDING)
	{
		throw ANKI_EXCEPTION("Problem with the pointLightsBlock");
	}

	ublock = &lightPassProg->findUniformBlock("spotLightsBlock");
	ublock->setBinding(SPOT_LIGHTS_BLOCK_BINDING);
	if(ublock->getSize() != sizeof(ShaderSpotLights)
		|| ublock->getBinding() != SPOT_LIGHTS_BLOCK_BINDING)
	{
		throw ANKI_EXCEPTION("Problem with the spotLightsBlock");
	}

	ublock = &lightPassProg->findUniformBlock("tilesBlock");
	ublock->setBinding(TILES_BLOCK_BINDING);
	if(ublock->getSize() != sizeof(ShaderTiles)
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

	Array<PointLight*, MAX_POINT_LIGHTS> visiblePointLights;
	U visiblePointLightsCount = 0;
	Array<SpotLight*, MAX_SPOT_LIGHTS> visibleSpotLights;
	U visibleSpotLightsCount = 0;

	U spotsNoShadowCount = 0, spotsShadowCount = 0;
	Array<SpotLight*, MAX_SPOT_LIGHTS> visibleSpotNoShadowLights, 
		visibleSpotShadowLights;

	//
	// Quickly get the lights
	//
	for(auto it = vi.getLightsBegin(); it != vi.getLightsEnd(); ++it)
	{
		Light* light = (*it)->getLight();
		switch(light->getLightType())
		{
		case Light::LT_POINT:
			// Use % to avoid overflows
			visiblePointLights[visiblePointLightsCount % MAX_POINT_LIGHTS] = 
				static_cast<PointLight*>(light);
			++visiblePointLightsCount;
			break;
		case Light::LT_SPOT:
			{
				SpotLight* slight = static_cast<SpotLight*>(light);
				
				if(slight->getShadowEnabled())
				{
					visibleSpotShadowLights[
						spotsShadowCount % MAX_SPOT_LIGHTS] = slight;
					++spotsShadowCount;
				}
				else
				{
					visibleSpotNoShadowLights[
						spotsNoShadowCount % MAX_SPOT_LIGHTS] = slight;
					++spotsNoShadowCount;
				}
				break;
			}
		case Light::LT_NUM:
			break;
		}
	}

	visibleSpotLightsCount = spotsShadowCount + spotsNoShadowCount;

	if(visiblePointLightsCount > MAX_POINT_LIGHTS 
		|| visibleSpotLightsCount > MAX_SPOT_LIGHTS)
	{
		throw ANKI_EXCEPTION("Too many visible lights");
	}

	for(U i = 0; i < spotsNoShadowCount; i++)
	{
		visibleSpotLights[i] = visibleSpotNoShadowLights[i];
	}

	for(U i = 0; i < spotsShadowCount; i++)
	{
		visibleSpotLights[i + spotsNoShadowCount] = visibleSpotShadowLights[i];
	}

	//
	// Do shadows pass
	//
	Array<Light*, Sm::MAX_SHADOW_CASTERS> shadowCasters;
	for(U i = 0; i < spotsShadowCount; i++)
	{
		shadowCasters[i] = visibleSpotShadowLights[i];
	}

	Array<U32, Sm::MAX_SHADOW_CASTERS> shadowmapLayers;
	sm.run(&shadowCasters[0], spotsShadowCount, shadowmapLayers);

	// Prepare state
	fbo.bind();
	r->clearAfterBindingFbo(
		GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	GlStateSingleton::get().setViewport(
		0, 0, r->getWidth(), r->getHeight());
	GlStateSingleton::get().disable(GL_DEPTH_TEST);
	GlStateSingleton::get().disable(GL_BLEND);

	//
	// Write the lights UBOs
	//

	// Map points
	ShaderPointLight* lightsMappedBuff = 
		(ShaderPointLight*)pointLightsUbo.map(
		0, 
		sizeof(ShaderPointLight) * visiblePointLightsCount,
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	WritePointLightsUbo jobs[ThreadPool::MAX_THREADS];
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		jobs[i].shaderLights = lightsMappedBuff;
		jobs[i].visibleLights = &visiblePointLights[0];
		jobs[i].visibleLightsCount = visiblePointLightsCount;
		jobs[i].is = this;

		threadPool.assignNewJob(i, &jobs[i]);
	}

	// Done
	threadPool.waitForAllJobsToFinish();
	pointLightsUbo.unmap();

	// Map spots
	ShaderSpotLight* shaderSpotLights = (ShaderSpotLight*)spotLightsUbo.map(
		0,
		sizeof(ShaderSpotLight) * visibleSpotLightsCount,
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	WriteSpotLightsUbo jobs2[ThreadPool::MAX_THREADS];
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		jobs2[i].shaderLights = shaderSpotLights;
		jobs2[i].visibleLights = &visibleSpotLights[0];
		jobs2[i].visibleLightsCount = visibleSpotLightsCount;
		jobs2[i].is = this;

		threadPool.assignNewJob(i, &jobs2[i]);
	}

	// Done
	threadPool.waitForAllJobsToFinish();

	// Set shadow layer IDs
	U32 i = 0;
	ShaderSpotLight* shaderSpotLight = shaderSpotLights + spotsNoShadowCount;
	for(; shaderSpotLight != shaderSpotLights + visibleSpotLightsCount;
		++shaderSpotLight)
	{
		shaderSpotLight->diffuseColorShadowmapId.w() = (F32)shadowmapLayers[i];
		++i;
	}

	spotLightsUbo.unmap();

	//
	// Update the tiles UBO
	//

	ShaderTile* stiles = (ShaderTile*)tilesUbo.map(
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	WriteTilesUboJob tjobs[ThreadPool::MAX_THREADS];
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		tjobs[i].visiblePointLights = &visiblePointLights[0];
		tjobs[i].visiblePointLightsCount = visiblePointLightsCount;

		tjobs[i].visibleSpotLights = &visibleSpotLights[0];
		tjobs[i].visibleSpotLightsCount = visibleSpotLightsCount;

		tjobs[i].shaderTiles = stiles;
		tjobs[i].is = this;

		threadPool.assignNewJob(i, &tjobs[i]);
	}

	threadPool.waitForAllJobsToFinish();
	tilesUbo.unmap();

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
		groundLightDir = cam->getViewMatrix().getColumn(1).xyz();
	}

	// Write common block
	if(commonUboUpdateTimestamp < r->getPlanesUpdateTimestamp()
		|| commonUboUpdateTimestamp < scene.getAmbientColorUpdateTimestamp()
		|| commonUboUpdateTimestamp == 1
		|| (groundLightEnabled
			&& !groundVectorsEqual(groundLightDir, prevGroundLightDir)))
	{
		ShaderCommonUniforms blk;
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
