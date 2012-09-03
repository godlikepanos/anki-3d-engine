#include "anki/renderer/Is.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/Scene.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Light.h"

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

typedef ShaderLight ShaderPointLight;

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
	Array<U32, Is::MAX_LIGHTS_PER_TILE> lightIndices;
};

struct ShaderTiles
{
	Array<Array<ShaderTile, Is::TILES_X_COUNT>, Is::TILES_Y_COUNT> tiles;
};

struct ShaderCommonUniforms
{
	Vec4 nearPlanes;
	Vec4 limitsOfNearPlane;
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
		"#define RENDERER_HEIGHT " + std::to_string(r->getWidth()) + "\n"
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
Bool Is::cullLight(const PointLight& plight, const Tile& tile)
{
	Camera& cam = r->getScene().getActiveCamera();
	Sphere sphere = plight.getSphere();

	//return cam.getFrustumable()->getFrustum().insideFrustum(sphere);

#if 1
	sphere.transform(Transform(cam.getViewMatrix()));

	for(const Plane& plane : tile.planes)
	{
		if(sphere.testPlane(plane) < 0.0)
		{
			return false;
		}
	}

	return true;
#endif
}

//==============================================================================
void Is::updateAllTilesPlanes(const PerspectiveCamera& cam)
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
		updateAllTilesPlanes(static_cast<const PerspectiveCamera&>(cam));
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
#if 1
	minMaxPassSprog->findUniformVariable("nearFar").set(
		Vec2(cam.getNear(), cam.getFar()));
	minMaxPassSprog->findUniformVariable("depthMap").set(
		r->getMs().getDepthFai());
#endif

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
#if 0
			F32 minZ =
				-r->getPlanes().y() / (r->getPlanes().x() + pixels[j][i][0]);
			F32 maxZ =
				-r->getPlanes().y() / (r->getPlanes().x() + pixels[j][i][1]);
#else
			F32 minZ = -cam.getNear();
			F32 maxZ = -cam.getFar();
#endif

			tile.planes[Frustum::FP_NEAR] = Plane(Vec3(0.0, 0.0, -1.0), -minZ);
			tile.planes[Frustum::FP_FAR] = Plane(Vec3(0.0, 0.0, 1.0), maxZ);
		}
	}
}

//==============================================================================
void Is::pointLightsPass()
{
	Camera& cam = r->getScene().getActiveCamera();
	VisibilityInfo& vi = cam.getFrustumable()->getVisibilityInfo();

	//
	// Write the lightsUbo
	//

	// Quickly count the point lights
	U pointLightsCount = 0;
	for(auto it = vi.getLightsBegin(); it != vi.getLightsEnd(); ++it)
	{
		const Light& light = *(*it);
		if(light.getLightType() == Light::LT_POINT)
		{
			++pointLightsCount;		
		}
	}

	if(pointLightsCount > MAX_LIGHTS)
	{
		throw ANKI_EXCEPTION("Too many lights");
	}

	// Map
	ShaderPointLights* lightsMappedBuff = (ShaderPointLights*)lightsUbo.map(
		0, 
		sizeof(ShaderPointLight) * pointLightsCount,
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	// Write the buff
	pointLightsCount = 0;
	for(auto it = vi.getLightsBegin(); it != vi.getLightsEnd(); ++it)
	{
		const Light& light = *(*it);
		if(light.getLightType() == Light::LT_POINT)
		{
			ShaderPointLight& pl = lightsMappedBuff->lights[pointLightsCount];
			const PointLight& plight = static_cast<const PointLight&>(light);

			Vec3 pos = light.getWorldTransform().getOrigin().getTransformed(
				cam.getViewMatrix());

			pl.posAndRadius = Vec4(pos, plight.getRadius());
			pl.diffuseColor = light.getDiffuseColor();
			pl.specularColor = light.getSpecularColor();

			++pointLightsCount;		
		}
	}

	// Done
	lightsUbo.unmap();

#if 1
	//
	// Update the tiles
	//

	// For all tiles write their indices 
	// OPT Parallelize that!!!!!!!!!!!!
	for(U j = 0; j < TILES_Y_COUNT; j++)
	{
		for(U i = 0; i < TILES_X_COUNT; i++)
		{
			Tile& tile = tiles[j][i];

			U lightsInTileCount = 0;
			U ids = 0;

			for(auto it = vi.getLightsBegin(); it != vi.getLightsEnd(); ++it)
			{
				const Light& light = *(*it);
				if(light.getLightType() != Light::LT_POINT)
				{
					continue;
				}

				const PointLight& plight =
					static_cast<const PointLight&>(light);

				if(cullLight(plight, tile))
				{
					tile.lightIndices[lightsInTileCount] = ids;
					++lightsInTileCount;
				}

				++ids;
			}

			/*if(lightsInTileCount > MAX_LIGHTS_PER_TILE)
			{
				throw ANKI_EXCEPTION("Too many lights per tile");
			}*/

			tile.lightsCount = lightsInTileCount;
		}
	}
#endif

	//
	// Write the tilesUbo
	//
	ShaderTiles* stiles = (ShaderTiles*)tilesUbo.map(
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	for(U j = 0; j < TILES_Y_COUNT; j++)
	{
		for(U i = 0; i < TILES_X_COUNT; i++)
		{
			const Tile& tile = tiles[j][i];
			stiles->tiles[j][i].lightsCount = tile.lightsCount;

			/*for(U k = 0; k < tile.lightsCount; k++)
			{
				stiles->tiles[j][i].lightIndices[k] = tile.lightIndices[k];
			}*/
		}
	}

	tilesUbo.unmap();
}

//==============================================================================
void Is::run()
{
	GlStateSingleton::get().disable(GL_DEPTH_TEST);
	GlStateSingleton::get().disable(GL_BLEND);

	updateTiles();

	fbo.bind();
	GlStateSingleton::get().setViewport(0, 0, r->getWidth(), r->getHeight());

	pointLightsPass();
	lightSProgs[0]->bind();
	r->drawQuadInstanced(TILES_Y_COUNT * TILES_X_COUNT);
}

} // end namespace anki
