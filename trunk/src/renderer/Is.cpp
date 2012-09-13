#include "anki/renderer/Is.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/Scene.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Light.h"
#include "anki/core/ThreadPool.h"

#define BLEND_ENABLE 1

namespace anki {

//==============================================================================

// Shader struct and block representations

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
	Mat4 texProjectionMat;
};

struct ShaderPointLights
{
	ShaderPointLight lights[Is::MAX_LIGHTS];
};

struct ShaderSpotLights
{
	ShaderSpotLight lights[Is::MAX_LIGHTS];
};

struct ShaderTile
{
	U32 lightsCount;
	U32 padding[3];
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
	ShaderPointLights* shaderLights; ///< Mapped UBO
	U32 maxShaderLights;
	PointLight** visibleLights;
	U32 visibleLightsCount;
	Is* is;

	void operator()(U threadId, U threadsCount)
	{
		U64 start, end;
		choseStartEnd(threadId, threadsCount, visibleLightsCount, start, end);
		is->writeLightUbo(*shaderLights, maxShaderLights, visibleLights, 
			visibleLightsCount, start, end);
	}
};

/// Job to update spot lights
struct WriteSpotLightsUbo: ThreadJob
{
	ShaderSpotLights* shaderLights; ///< Mapped UBO
	U32 maxShaderLights;
	SpotLight** visibleLights;
	U32 visibleLightsCount;
	Is* is;

	void operator()(U threadId, U threadsCount)
	{
		U64 start, end;
		choseStartEnd(threadId, threadsCount, visibleLightsCount, start, end);
		is->writeLightUbo(*shaderLights, maxShaderLights, visibleLights, 
			visibleLightsCount, start, end);
	}
};

/// A job to write the tiles UBO
template<typename TLight>
struct WriteTilesUboJob: ThreadJob
{
	TLight** visibleLights;
	U32 visibleLightsCount;
	ShaderTiles* shaderTiles;
	U32 maxLightsPerTile;
	Is* is;

	void operator()(U threadId, U threadsCount)
	{
		U64 start, end;
		choseStartEnd(threadId, threadsCount, 
			Is::TILES_X_COUNT * Is::TILES_Y_COUNT, start, end);

		is->writeTilesUbo(visibleLights, visibleLightsCount,
			*shaderTiles, maxLightsPerTile, start, end);
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
		"#define TILES_COUNT " + std::to_string(TILES_X_COUNT * TILES_Y_COUNT)
		+ "\n"
		"#define MAX_LIGHTS " + std::to_string(MAX_LIGHTS) + "\n";

	// point light
	lightSProgs[LST_POINT].load(ShaderProgramResource::createSrcCodeToCache(
		"shaders/IsLpGeneric.glsl",
		(pps + "#define POINT_LIGHT 1\n").c_str()).c_str());

	// spot light no shadow
	lightSProgs[LST_SPOT].load(ShaderProgramResource::createSrcCodeToCache(
		"shaders/IsLpGeneric.glsl",
		(pps + "#define SPOT_LIGHT 1\n").c_str()).c_str());

	// spot light w/t shadow
	std::string pps2 = pps + "#define SPOT_LIGHT 1\n"
		"#define SHADOW 1\n";
	if(/*sm.isPcfEnabled()*/ 1) // XXX
	{
		pps2 += "#define PCF 1\n";
	}
	lightSProgs[LST_SPOT_SHADOW].load(
		ShaderProgramResource::createSrcCodeToCache(
		"shaders/IsLpGeneric.glsl", pps2.c_str()).c_str());

	// Min max
	std::string filename = ShaderProgramResource::createSrcCodeToCache(
		"shaders/IsMinMax.glsl",
		pps.c_str());
	minMaxPassSprog.load(filename.c_str());

	//
	// Create FBOs
	//

	// IS FBO
	Renderer::createFai(r->getWidth(), r->getHeight(), GL_RGB8,
		GL_RGB, GL_UNSIGNED_INT, fai);
	fbo.create();
	fbo.setColorAttachments({&fai});

	if(!fbo.isComplete())
	{
		throw ANKI_EXCEPTION("Fbo not complete");
	}

	// min max FBO
	Renderer::createFai(TILES_X_COUNT, TILES_Y_COUNT, GL_RG32F, GL_RG,
		GL_FLOAT, minMaxFai);
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
	/*const ShaderProgramUniformBlock& block =
		lightSProgs[LST_POINT]->findUniformBlock("commonBlock");*/

	commonUbo.create(sizeof(ShaderCommonUniforms), nullptr);
	commonUbo.setBinding(0);

	// lights UBO
	lightsUbo.create(sizeof(ShaderSpotLights), nullptr);
	lightsUbo.setBinding(1);

	// lightIndices UBO
	tilesUbo.create(sizeof(ShaderTiles), nullptr);
	tilesUbo.setBinding(2);

	// Sanity checks
	const ShaderProgramUniformBlock* ublock;

	ublock = &lightSProgs[LST_POINT]->findUniformBlock("commonBlock");
	if(ublock->getSize() != sizeof(ShaderCommonUniforms)
		|| ublock->getBindingPoint() != 0)
	{
		throw ANKI_EXCEPTION("Problem with the commonBlock");
	}

	ublock = &lightSProgs[LST_POINT]->findUniformBlock("lightsBlock");
	if(ublock->getSize() != sizeof(ShaderPointLights)
		|| ublock->getBindingPoint() != 1)
	{
		throw ANKI_EXCEPTION("Problem with the lightsBlock");
	}

	ublock = &lightSProgs[LST_POINT]->findUniformBlock("tilesBlock");
	if(ublock->getSize() != sizeof(ShaderTiles)
		|| ublock->getBindingPoint() != 2)
	{
		throw ANKI_EXCEPTION("Problem with the tilesBlock");
	}
}

//==============================================================================
Bool Is::cullLight(const PointLight& plight, const Tile& tile, 
	const Mat4& viewMatrix)
{
	Sphere sphere = plight.getSphere();
	sphere.transform(Transform(viewMatrix));

	for(const Plane& plane : tile.planes)
	{
		if(sphere.testPlane(plane) < 0.0)
		{
			return false;
		}
	}

	return true;
}

//==============================================================================
void Is::updateAllTilesPlanesInternal(const PerspectiveCamera& cam)
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

	for(U j = 0; j < TILES_Y_COUNT; j++)
	{
		for(U i = 0; i < TILES_X_COUNT; i++)
		{
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
}

//==============================================================================
void Is::updateAllTilesPlanes()
{
	Camera& cam = r->getScene().getActiveCamera();
	U32 camTimestamp = cam.getFrustumable()->getFrustumableTimestamp();

	if(camTimestamp < planesUpdateTimestamp)
	{
		return;
	}

	switch(cam.getCameraType())
	{
	case Camera::CT_PERSPECTIVE:
		updateAllTilesPlanesInternal(
			static_cast<const PerspectiveCamera&>(cam));
		break;
	default:
		ANKI_ASSERT(0 && "Unimplemented");
		break;
	}

	planesUpdateTimestamp = Timestamp::getTimestamp();
}

//==============================================================================
void Is::updateTiles()
{
	// Do the min/max pass
	//
	const Camera& cam = r->getScene().getActiveCamera();

	minMaxTilerFbo.bind();
	minMaxPassSprog->bind();
	GlStateSingleton::get().setViewport(0, 0, TILES_X_COUNT, TILES_Y_COUNT);

	minMaxPassSprog->findUniformVariable("depthMap").set(
		r->getMs().getDepthFai());

	r->drawQuad();

	// Do something else instead of waiting for the draw call to finish.
	// Update the tile planes
	//
	updateAllTilesPlanes();

	// Update the near and far planes of tiles
	//
	F32 pixels[TILES_Y_COUNT][TILES_X_COUNT][2];
	minMaxFai.readPixels(pixels);

	for(U j = 0; j < TILES_Y_COUNT; j++)
	{
		for(U i = 0; i < TILES_X_COUNT; i++)
		{
			Tile& tile = tiles[j][i];

			/// Calculate as you do in the vertex position inside the shaders
			F32 minZ =
				-r->getPlanes().y() / (r->getPlanes().x() + pixels[j][i][0]);
			F32 maxZ =
				-r->getPlanes().y() / (r->getPlanes().x() + pixels[j][i][1]);

			tile.planes[Frustum::FP_NEAR] = Plane(Vec3(0.0, 0.0, -1.0), -minZ);
			tile.planes[Frustum::FP_FAR] = Plane(Vec3(0.0, 0.0, 1.0), maxZ);
		}
	}
}

//==============================================================================
void Is::writeLightUbo(ShaderPointLights& shaderLights, U32 maxShaderLights,
	PointLight** visibleLights, U32 visibleLightsCount, U start, U end)
{
	const Camera& cam = r->getScene().getActiveCamera();

	for(U64 i = start; i < end; i++)
	{
		ANKI_ASSERT(i < maxShaderLights);
		ShaderPointLight& pl = shaderLights.lights[i];
		const PointLight& light = *visibleLights[i];

		Vec3 pos = light.getWorldTransform().getOrigin().getTransformed(
			cam.getViewMatrix());

		pl.posAndRadius = Vec4(pos, light.getRadius());
		pl.diffuseColor = light.getDiffuseColor();
		pl.specularColor = light.getSpecularColor();
	}
}

//==============================================================================
void Is::writeLightUbo(ShaderSpotLight& shaderLights, U32 maxShaderLights,
	SpotLight** visibleLights, U32 visibleLightsCount, U start, U end)
{
	const Camera& cam = r->getScene().getActiveCamera();

	for(U64 i = start; i < end; i++)
	{
		ANKI_ASSERT(i < maxShaderLights);
		ShaderSpotLight& slight = shaderLights.lights[i];
		const SpotLight& light = *visibleLights[i];

		Vec3 pos = light.getWorldTransform().getOrigin().getTransformed(
			cam.getViewMatrix());

		slight.posAndRadius = Vec4(pos, light.getDistance());
		slight.diffuseColor = light.getDiffuseColor();
		slight.specularColor = light.getSpecularColor();
		
		static const Mat4 biasMat4(0.5, 0.0, 0.0, 0.5, 0.0, 0.5, 0.0, 0.5, 
			0.0, 0.0, 0.5, 0.5, 0.0, 0.0, 0.0, 1.0);
		slight.texProjectionMat = biasMat4 * light.getProjectionMatrix() *
			Mat4::combineTransformations(light.getViewMatrix(),
			Mat4(cam.getWorldTransform()));
	}
}

//==============================================================================
template<typename TLight>
void Is::writeTilesUbo(TLight** visibleLights, U32 visibleLightsCount,
	ShaderTiles& shaderTiles, U32 maxLightsPerTile,
	U32 start, U32 end)
{
	Tile* tiles_ = &tiles[0][0];
	const Camera& cam = r->getScene().getActiveCamera();
	ANKI_ASSERT(maxLightsPerTile <= MAX_LIGHTS_PER_TILE);

	for(U32 i = start; i < end; i++)
	{
		ANKI_ASSERT(i < TILES_X_COUNT * TILES_Y_COUNT);

		Tile& tile = tiles_[i];
		Array<U32, MAX_LIGHTS_PER_TILE> lightIndices;
		U lightsInTileCount = 0;

		for(U j = 0; j < visibleLightsCount; j++)
		{
			const TLight& light = *visibleLights[j];

			if(cullLight(light, tile, cam.getViewMatrix()))
			{
				// Use % to avoid overflows
				lightIndices[lightsInTileCount % maxLightsPerTile] = j;
				++lightsInTileCount;
			}
		}
#if 0
		if(lightsInTileCount > maxLightsPerTile)
		{
			ANKI_LOGW("Too many lights per tile: " << lightsInTileCount);
		}
#endif
		shaderTiless.tiles[i].lightsCount = lightsInTileCount;

		memcpy(
			&(shaderTiles.tiles[i].lightIndices[0]),
			&lightIndices[0],
			sizeof(U32) * lightsInTileCount);
	}
}

//==============================================================================
void Is::pointLightsPass()
{
	Camera& cam = r->getScene().getActiveCamera();
	VisibilityInfo& vi = cam.getFrustumable()->getVisibilityInfo();

	Array<PointLight*, MAX_LIGHTS> visibleLights;
	U visibleLightsCount = 0;

	ThreadPool& threadPool = ThreadPoolSingleton::get();

	//
	// Write the lightsUbo
	//

	// Quickly get the point lights
	for(auto it = vi.getLightsBegin(); it != vi.getLightsEnd(); ++it)
	{
		Light* light = (*it);
		if(light->getLightType() == Light::LT_POINT)
		{
			// Use % to avoid overflows
			visibleLights[visibleLightsCount % MAX_LIGHTS] = 
				static_cast<PointLight*>(light);
			++visibleLightsCount;
		}
	}

	if(visibleLightsCount > MAX_LIGHTS)
	{
		throw ANKI_EXCEPTION("Too many visible lights");
	}

	// Map
	ShaderPointLights* lightsMappedBuff = (ShaderPointLights*)lightsUbo.map(
		0, 
		sizeof(ShaderPointLight) * visibleLightsCount,
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	WritePointLightsUbo jobs[ThreadPool::MAX_THREADS];
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		jobs[i].shaderLights = lightsMappedBuff;
		jobs[i].maxShaderLights = MAX_LIGHTS;
		jobs[i].visibleLights = &visibleLights[0];
		jobs[i].visibleLightsCount = visibleLightsCount;
		jobs[i].is = this;

		threadPool.assignNewJob(i, &jobs[i]);
	}

	// Done
	threadPool.waitForAllJobsToFinish();
	lightsUbo.unmap();

	//
	// Update the tiles
	//

	ShaderTiles* stiles = (ShaderTiles*)tilesUbo.map(
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	WriteTilesUboJob<PointLight> tjobs[ThreadPool::MAX_THREADS];
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		tjobs[i].visibleLights = &visibleLights[0];
		tjobs[i].visibleLightsCount = visibleLightsCount;
		tjobs[i].shaderTiles = stiles;
		tjobs[i].maxLightsPerTile = MAX_LIGHTS_PER_TILE;
		tjobs[i].is = this;

		threadPool.assignNewJob(i, &tjobs[i]);
	}

	threadPool.waitForAllJobsToFinish();
	tilesUbo.unmap();

	//
	// Setup shader and draw
	//
	
	// shader prog
	const ShaderProgram& shader = *lightSProgs[LST_POINT];
	shader.bind();

	shader.findUniformVariable("msFai0").set(r->getMs().getFai0());
	shader.findUniformVariable("msDepthFai").set(
		r->getMs().getDepthFai());

	r->drawQuadInstanced(TILES_Y_COUNT * TILES_X_COUNT);
}

//==============================================================================
void Is::spotLightsPass(Bool shadow)
{
	Camera& cam = r->getScene().getActiveCamera();
	VisibilityInfo& vi = cam.getFrustumable()->getVisibilityInfo();

	Array<SpotLight*, MAX_LIGHTS> visibleLights;
	U visibleLightsCount = 0;

	ThreadPool& threadPool = ThreadPoolSingleton::get();

	// Quickly get the point lights
	for(auto it = vi.getLightsBegin(); it != vi.getLightsEnd(); ++it)
	{
		Light* light = (*it);
		if(light->getLightType() == Light::LT_SPOT)
		{
			SpotLight* slight = static_cast<SpotLight*>(light);

			if(shadow && slight->getShadowEnabled())
			{
				// Use % to avoid overflows
				visibleLights[visibleLightsCount % MAX_LIGHTS] = slight;
				++visibleLightsCount;
			}
		}
	}

	if(visibleLightsCount > MAX_LIGHTS)
	{
		throw ANKI_EXCEPTION("Too many visible lights");
	}

	//
	// Write the lightsUbo
	//

	ShaderSpotLights* lightsMappedBuff = (ShaderSpotLights*)lightsUbo.map(
		0, 
		sizeof(ShaderSpotLight) * visibleLightsCount,
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	WriteSpotLightsUbo jobs[ThreadPool::MAX_THREADS];
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		jobs[i].shaderLights = lightsMappedBuff;
		jobs[i].maxShaderLights = MAX_LIGHTS;
		jobs[i].visibleLights = &visibleLights[0];
		jobs[i].visibleLightsCount = visibleLightsCount;
		jobs[i].is = this;

		threadPool.assignNewJob(i, &jobs[i]);
	}

	// Done
	threadPool.waitForAllJobsToFinish();
	lightsUbo.unmap();

	//
	// Update the tiles
	//

	ShaderTiles* stiles = (ShaderTiles*)tilesUbo.map(
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	WriteTilesUboJob<SpotLight> tjobs[ThreadPool::MAX_THREADS];
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		tjobs[i].visibleLights = &visibleLights[0];
		tjobs[i].visibleLightsCount = visibleLightsCount;
		tjobs[i].shaderTiles = stiles;
		tjobs[i].maxLightsPerTile = MAX_LIGHTS_PER_TILE;
		tjobs[i].is = this;

		threadPool.assignNewJob(i, &tjobs[i]);
	}

	threadPool.waitForAllJobsToFinish();
	tilesUbo.unmap();
	
	//
	// Setup shader and draw
	//
	
	// shader prog
	const ShaderProgram& shader = *lightSProgs[
		(shadow) ? LST_SPOT_SHADOW : LST_SPOT];
	shader.bind();

	shader.findUniformVariable("msFai0").set(r->getMs().getFai0());
	shader.findUniformVariable("msDepthFai").set(
		r->getMs().getDepthFai());

	r->drawQuadInstanced(TILES_Y_COUNT * TILES_X_COUNT);
}

//==============================================================================
void Is::run()
{
	GlStateSingleton::get().disable(GL_DEPTH_TEST);
	GlStateSingleton::get().disable(GL_BLEND);

	// Write common block
	Scene& scene = r->getScene();
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
		blk.sceneAmbientColor = Vec4(r->getScene().getAmbientColor(), 0.0);

		commonUbo.write(&blk);

		commonUboUpdateTimestamp = Timestamp::getTimestamp();
	}

	commonUbo.setBinding(0);
	lightsUbo.setBinding(1);
	tilesUbo.setBinding(2);

	// Update tiles
	updateTiles();

	// Do the rest
	fbo.bind();
	GlStateSingleton::get().setViewport(0, 0, r->getWidth(), r->getHeight());

	pointLightsPass();

	GlStateSingleton::get().enable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	spotLightsPass(false);
}

} // end namespace anki
