#include "anki/renderer/Is.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/Scene.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Light.h"
#include "anki/core/ThreadPool.h"

namespace anki {

//==============================================================================

// Shader structs and block representations

struct ShaderLight
{
	Vec4 posAndRadius; ///< xyz: Light pos in eye space. w: The radius
	Vec4 diffuseColor;
	Vec4 specularColor;
};

struct ShaderPointLight: ShaderLight
{};

struct ShaderSpotLight: ShaderLight
{
	Vec4 lightDirection;
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
	Array<ShaderTile, Is::TILES_X_COUNT * Is::TILES_Y_COUNT> tiles;
};

struct ShaderCommonUniforms
{
	Vec4 nearPlanes;
	Vec4 limitsOfNearPlane;
	Vec4 sceneAmbientColor;
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
			pl.diffuseColor = light.getDiffuseColor();
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

			slight.diffuseColor = Vec4(light.getDiffuseColor().xyz(),
				light.getOuterAngleCos());

			slight.specularColor = Vec4(light.getSpecularColor().xyz(),
				light.getInnerAngleCos());

			Vec3 lightDir = -light.getWorldTransform().getRotation().getZAxis();
			lightDir = cam->getViewMatrix().getRotationPart() * lightDir;
			slight.lightDirection = Vec4(lightDir, 0.0);
			
			static const Mat4 biasMat4(
				0.5, 0.0, 0.0, 0.5, 
				0.0, 0.5, 0.0, 0.5, 
				0.0, 0.0, 0.5, 0.5, 
				0.0, 0.0, 0.0, 1.0);
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
			Is::TILES_X_COUNT * Is::TILES_Y_COUNT, start, end);

		for(U32 i = start; i < end; i++)
		{
			Is::Tile& tile = is->tiles1d[i];
			ShaderTile& stile = shaderTiles[i];

			doTile(tile, stile);
		}
	}

	/// Do a tile
	void doTile(Is::Tile& tile, ShaderTile& stile)
	{
		auto& lightIndices = stile.lightIndices;

		// Point lights
		//

		U pointLightsInTileCount = 0;
		for(U i = 0; i < visiblePointLightsCount; i++)
		{
			const PointLight& light = *visiblePointLights[i];

			if(Is::cullLight(light, tile))
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

			if(Is::cullLight(light, tile))
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

/// Job that updates the tile planes
struct UpdateTilesJob: ThreadJob
{
	F32 (*pixels)[Is::TILES_Y_COUNT][Is::TILES_X_COUNT][2];
	Is* is;

	void operator()(U threadId, U threadsCount)
	{
		U64 start, end;
		choseStartEnd(threadId, threadsCount, 
			Is::TILES_X_COUNT * Is::TILES_Y_COUNT, start, end);

		is->updateTilePlanes(pixels, start, end);
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
		"#define RENDERER_WIDTH " + std::to_string(r->getWidth()) + "\n"
		"#define RENDERER_HEIGHT " + std::to_string(r->getHeight()) + "\n"
		"#define MAX_LIGHTS_PER_TILE " + std::to_string(MAX_LIGHTS_PER_TILE)
		+ "\n"
		"#define MAX_POINT_LIGHTS " + std::to_string(MAX_POINT_LIGHTS) + "\n"
		"#define MAX_SPOT_LIGHTS " + std::to_string(MAX_SPOT_LIGHTS) + "\n";

	if(sm.getPcfEnabled())
	{
		pps += "#define PCF 1\n";
	}

	// point light
	lightPassProg.load(ShaderProgramResource::createSrcCodeToCache(
		"shaders/IsLpGeneric.glsl", pps.c_str()).c_str());

	// Min max
	minMaxPassSprog.load(ShaderProgramResource::createSrcCodeToCache(
		"shaders/IsMinMax.glsl", pps.c_str()).c_str());

	//
	// Create FBOs
	//

	// IS FBO
	Renderer::createFai(r->getWidth(), r->getHeight(), GL_RGB8,
		GL_RGB, GL_UNSIGNED_INT, fai);
	fbo.create();
	fbo.setColorAttachments({&fai});
	fbo.setOtherAttachment(GL_DEPTH_ATTACHMENT, r->getMs().getDepthFai());

	if(!fbo.isComplete())
	{
		throw ANKI_EXCEPTION("Fbo not complete");
	}

	// min max FBO
	Renderer::createFai(TILES_X_COUNT, TILES_Y_COUNT, GL_RG32UI,
		GL_RG_INTEGER, GL_UNSIGNED_INT, minMaxFai);
	minMaxFai.setFiltering(Texture::TFT_NEAREST);
	minMaxTilerFbo.create();
	minMaxTilerFbo.setColorAttachments({&minMaxFai});
	if(!minMaxTilerFbo.isComplete())
	{
		throw ANKI_EXCEPTION("minMaxTilerFbo not complete");
	}

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
	if(ublock->getSize() != sizeof(ShaderCommonUniforms)
		|| ublock->getBindingPoint() != COMMON_UNIFORMS_BLOCK_BINDING)
	{
		throw ANKI_EXCEPTION("Problem with the commonBlock");
	}

	ublock = &lightPassProg->findUniformBlock("pointLightsBlock");
	if(ublock->getSize() != sizeof(ShaderPointLights)
		|| ublock->getBindingPoint() != POINT_LIGHTS_BLOCK_BINDING)
	{
		throw ANKI_EXCEPTION("Problem with the pointLightsBlock");
	}

	ublock = &lightPassProg->findUniformBlock("spotLightsBlock");
	if(ublock->getSize() != sizeof(ShaderSpotLights)
		|| ublock->getBindingPoint() != SPOT_LIGHTS_BLOCK_BINDING)
	{
		throw ANKI_EXCEPTION("Problem with the spotLightsBlock");
	}

	ublock = &lightPassProg->findUniformBlock("spotLightsBlock");
	if(ublock->getSize() != sizeof(ShaderSpotLights)
		|| ublock->getBindingPoint() != SPOT_LIGHTS_BLOCK_BINDING)
	{
		throw ANKI_EXCEPTION("Problem with the spotLightsBlock");
	}

	ublock = &lightPassProg->findUniformBlock("tilesBlock");
	if(ublock->getSize() != sizeof(ShaderTiles)
		|| ublock->getBindingPoint() != TILES_BLOCK_BINDING)
	{
		throw ANKI_EXCEPTION("Problem with the tilesBlock");
	}
}

//==============================================================================
Bool Is::cullLight(const PointLight& plight, const Tile& tile)
{
	const Sphere& sphere = plight.getSphere();

	for(const Plane& plane : tile.planesWSpace)
	{
		if(sphere.testPlane(plane) < 0.0)
		{
			return false;
		}
	}

	return true;
}

//==============================================================================
Bool Is::cullLight(const SpotLight& light, const Tile& tile)
{
	const PerspectiveFrustum& fr = light.getFrustum();

	for(const Plane& plane : tile.planesWSpace)
	{
		if(fr.testPlane(plane) < 0.0)
		{
			return false;
		}
	}

	return true;
}

//==============================================================================
void Is::updateTiles()
{
	// Do the min/max pass
	//
	minMaxTilerFbo.bind();
	minMaxPassSprog->bind();
	GlStateSingleton::get().setViewport(0, 0, TILES_X_COUNT, TILES_Y_COUNT);

	minMaxPassSprog->findUniformVariable("depthMap").set(
		r->getMs().getDepthFai());

	r->drawQuad();

	F32 pixels[TILES_Y_COUNT][TILES_X_COUNT][2];
#if ANKI_GL == ANKI_GL_DESKTOP
	// It seems read from texture is a bit faster than readpixels on nVidia
	minMaxFai.readPixels(pixels);
#else
	glReadPixels(0, 0, TILES_X_COUNT, TILES_Y_COUNT, GL_RG_INTEGER,
		GL_UNSIGNED_INT, &pixels[0][0][0]);
#endif

	// Update the rest of the tile stuff in parallel
	// 

	ThreadPool& threadPool = ThreadPoolSingleton::get();
	UpdateTilesJob jobs[ThreadPool::MAX_THREADS];
	
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		jobs[i].pixels = &pixels;
		jobs[i].is = this;

		threadPool.assignNewJob(i, &jobs[i]);
	}

	threadPool.waitForAllJobsToFinish();
}

//==============================================================================
void Is::updateTilePlanes(F32 (*pixels)[TILES_Y_COUNT][TILES_X_COUNT][2],
	U32 start, U32 stop)
{
	// Update only the 4 planes
	updateTiles4Planes(start, stop);

	// - Calc the rest of the 2 planes and 
	// - transform the planes
	for(U32 k = start; k < stop; k++)
	{
		U i = k % TILES_X_COUNT;
		U j = k / TILES_X_COUNT;
		Tile& tile = tiles[j][i];

		/// Calculate as you do in the vertex position inside the shaders
		F32 minZ = 
			r->getPlanes().y() / (r->getPlanes().x() + (*pixels)[j][i][0]);
		F32 maxZ = 
			-r->getPlanes().y() / (r->getPlanes().x() + (*pixels)[j][i][1]);

		tile.planes[Frustum::FP_NEAR] = Plane(Vec3(0.0, 0.0, -1.0), minZ);
		tile.planes[Frustum::FP_FAR] = Plane(Vec3(0.0, 0.0, 1.0), maxZ);

		// Now transform
		for(U k = 0; k < 6; k++)
		{
			tile.planesWSpace[k] = tile.planes[k].getTransformed(
				Transform(cam->getWorldTransform()));
		}
	}
}

//==============================================================================
void Is::updateTiles4Planes(U32 start, U32 stop)
{
	U32 camTimestamp = cam->getFrustumable()->getFrustumableTimestamp();
	if(camTimestamp < planesUpdateTimestamp)
	{
		// Early exit if the frustum have not changed
		return;
	}

	switch(cam->getCameraType())
	{
	case Camera::CT_PERSPECTIVE:
		updateTiles4PlanesInternal(
			static_cast<const PerspectiveCamera&>(*cam), start, stop);
		break;
	default:
		ANKI_ASSERT(0 && "Unimplemented");
		break;
	}

	planesUpdateTimestamp = Timestamp::getTimestamp();
}

//==============================================================================
void Is::updateTiles4PlanesInternal(const PerspectiveCamera& cam,
	U32 start, U32 stop)
{
	// The algorithm is almost the same as the recalculation of planes for
	// PerspectiveFrustum class

	// Algorithm works for even number of tiles per dimension
	ANKI_ASSERT(TILES_X_COUNT % 2 == 0 && TILES_Y_COUNT % 2 == 0);

	F32 fx = cam.getFovX();
	F32 fy = cam.getFovY();
	F32 n = cam.getNear();

	F32 l = 2.0 * n * tan(fx / 2.0);
	F32 l6 = l / TILES_X_COUNT;
	F32 o = 2.0 * n * tan(fy / 2.0);
	F32 o6 = o / TILES_Y_COUNT;

	for(U32 k = start; k < stop; k++)
	{
		U i = k % TILES_X_COUNT;
		U j = k / TILES_X_COUNT;

		Array<Plane, Frustum::FP_COUNT>& planes = tiles[j][i].planes;
		Vec3 a, b;

		// left
		a = Vec3((I(i) - I(TILES_X_COUNT) / 2) * l6, 0.0, -n);
		b = a.cross(Vec3(0.0, 1.0, 0.0));
		b.normalize();

		planes[Frustum::FP_LEFT] = Plane(b, 0.0);

		// right
		a = Vec3((I(i) - I(TILES_X_COUNT) / 2 + 1) * l6, 0.0, -n);
		b = Vec3(0.0, 1.0, 0.0).cross(a);
		b.normalize();

		planes[Frustum::FP_RIGHT] = Plane(b, 0.0);

		// bottom
		a = Vec3(0.0, (I(j) - I(TILES_Y_COUNT) / 2) * o6, -n);
		b = Vec3(1.0, 0.0, 0.0).cross(a);
		b.normalize();

		planes[Frustum::FP_BOTTOM] = Plane(b, 0.0);

		// bottom
		a = Vec3(0.0, (I(j) - I(TILES_Y_COUNT) / 2 + 1) * o6, -n);
		b = a.cross(Vec3(1.0, 0.0, 0.0));
		b.normalize();

		planes[Frustum::FP_TOP] = Plane(b, 0.0);
	}
}

//==============================================================================
void Is::lightPass()
{
	ThreadPool& threadPool = ThreadPoolSingleton::get();
	VisibilityInfo& vi = cam->getFrustumable()->getVisibilityInfo();

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

	Array<Texture*, Sm::MAX_SHADOW_CASTERS> shadowmaps;
	sm.run(&shadowCasters[0], spotsShadowCount, &shadowmaps[0]);

	// Prepare state
	fbo.bind();
	GlStateSingleton::get().setViewport(0, 0, r->getWidth(), r->getHeight());
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

	lightPassProg->findUniformVariable("msFai0").set(r->getMs().getFai0());
	lightPassProg->findUniformVariable("msDepthFai").set(
		r->getMs().getDepthFai());

#if 1
	lightPassProg->findUniformVariable("shadowMaps[0]").set(
		&shadowmaps[0], (U32)spotsShadowCount);
#else
	for(U i = 0; i < spotsShadowCount; i++)
	{
		char str[128];
		sprintf(str, "shadowMaps[%u]", (unsigned int)i);

		lightPassProg->findUniformVariable(str).set(*shadowmaps[i]);
	}
#endif

	r->drawQuadInstanced(TILES_Y_COUNT * TILES_X_COUNT);
}

//==============================================================================
void Is::run()
{
	Scene& scene = r->getScene();
	cam = &scene.getActiveCamera();

	// Write common block
	if(commonUboUpdateTimestamp < r->getPlanesUpdateTimestamp()
		|| commonUboUpdateTimestamp < scene.getAmbientColorUpdateTimestamp()
		|| commonUboUpdateTimestamp == 1)
	{
		const Camera& cam = scene.getActiveCamera();
		ShaderCommonUniforms blk;
		blk.nearPlanes = Vec4(cam.getNear(), 0.0, r->getPlanes().x(),
			r->getPlanes().y());
		blk.limitsOfNearPlane = Vec4(r->getLimitsOfNearPlane(),
			r->getLimitsOfNearPlane2());
		blk.sceneAmbientColor = Vec4(scene.getAmbientColor(), 0.0);

		commonUbo.write(&blk);

		commonUboUpdateTimestamp = Timestamp::getTimestamp();
	}

	commonUbo.setBinding(COMMON_UNIFORMS_BLOCK_BINDING);
	pointLightsUbo.setBinding(POINT_LIGHTS_BLOCK_BINDING);
	spotLightsUbo.setBinding(SPOT_LIGHTS_BLOCK_BINDING);
	tilesUbo.setBinding(TILES_BLOCK_BINDING);

	// Update tiles
	updateTiles();

	// Do the light pass including the shadow passes
	lightPass();
}

} // end namespace anki
