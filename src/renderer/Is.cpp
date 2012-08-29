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
		"#define RENDERER_HEIGHT " + std::to_string(r->getWidth()) + "\n";

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
	commonUbo.setBinding(1);

	// lightIndices UBO
	lightIndicesUbo.create(sizeof(ShaderTiles), nullptr);
	commonUbo.setBinding(2);

	//
	// Init tiles
	//
	F32 tileW = 1.0 / TILES_X_COUNT;
	F32 tileH = 1.0 / TILES_Y_COUNT;

	for(U j = 0; j < TILES_Y_COUNT; j++)
	{
		for(U i = 0; i < TILES_X_COUNT; i++)
		{
			F32 x = i * tileW;
			F32 y = j * tileH;

			tiles[j][i].coords[0] = Vec2(x, y) * 2.0 - 1.0;
			tiles[j][i].coords[1] = Vec2(x + tileW, y + tileH) * 2.0 - 1.0;
		}
	}
}

//==============================================================================
void Is::projectShape(const Camera& cam,
	const Sphere& sphere, Vec2& circleCenter, F32& circleRadius)
{
	F32 r = sphere.getRadius();
	Vec4 c(sphere.getCenter(), 1.0);
	Vec4 a = Vec4(sphere.getCenter()
		+ cam.getWorldTransform().getRotation() * Vec3(r, 0.0, 0.0), 1.0);

	c = cam.getViewProjectionMatrix() * c;
	c /= c.w();

	a = cam.getViewProjectionMatrix() * a;
	a /= a.w();

	circleCenter = c.xy();
	circleRadius = (c.xy() - a.xy()).getLength();
}

//==============================================================================
Bool Is::circleIntersects(const Tile& tile, const Vec2& c, F32 r)
{
	// For more info about the algorithm see the collision between an AABB and
	// a sphere

	Vec2 cp; // Closes point
	for(U i = 0; i < 2; i++)
	{
		if(c[i] > tile.coords[1][i])
		{
			cp[i] = tile.coords[1][i];
		}
		else if(c[i] < tile.coords[0][i])
		{
			cp[i] = tile.coords[0][i];
		}
		else
		{
			cp[i] = c[i];
		}
	}

	F32 rsq = r * r;
	Vec2 sub = c - cp;

	if(sub.getLengthSquared() <= rsq)
	{
		return true;
	}

	return false;
}

//==============================================================================
Bool Is::cullLight(const PointLight& plight, const Tile& tile)
{
	Vec2 cc;
	F32 rad;
	projectShape(r->getScene().getActiveCamera(), plight.getSphere(), cc, rad);
	return circleIntersects(tile, cc, rad);
}

//==============================================================================
void Is::minMaxPass()
{
	// Do the pass
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

	// Update the tiles
	//
	F32 pixels[TILES_Y_COUNT][TILES_X_COUNT][2];

	minMaxFai.readPixels(pixels);

	for(U j = 0; j < TILES_Y_COUNT; j++)
	{
		for(U i = 0; i < TILES_X_COUNT; i++)
		{
			Tile& tile = tiles[j][i];
			tile.depth = Vec2(pixels[j][i][0], pixels[j][i][1]);
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

	// Map
	ShaderPointLight* lightsMappedBuff = (ShaderPointLight*)lightsUbo.map(
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
			ShaderPointLight& pl = lightsMappedBuff[pointLightsCount];
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

#if 0
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
					tile.lightIndices[ids] = lightsInTileCount;
					++ids;
				}

				++lightsInTileCount;
			}

			tile.lightsCount = lightsInTileCount;
		}
	}
#endif

	//
	// Write the lightIndicesUbo
	//

	ShaderTiles* stiles = (ShaderTiles*)lightIndicesUbo.map(
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	for(U j = 0; j < TILES_Y_COUNT; j++)
	{
		for(U i = 0; i < TILES_X_COUNT; i++)
		{
			const Tile& tile = tiles[j][i];
			stiles->tiles[j][i].lightsCount = tile.lightsCount;

			for(U k = 0; k < tile.lightsCount; k++)
			{
				stiles->tiles[j][i].lightIndices[k] = tile.lightIndices[k];
			}
		}
	}

	lightIndicesUbo.unmap();
}

//==============================================================================
void Is::run()
{
	GlStateSingleton::get().disable(GL_DEPTH_TEST);
	GlStateSingleton::get().disable(GL_BLEND);

	minMaxPass();

	fbo.bind();
	GlStateSingleton::get().setViewport(0, 0, r->getWidth(), r->getHeight());

	pointLightsPass();
	lightSProgs[0]->bind();
	r->drawQuadInstanced(TILES_Y_COUNT * TILES_X_COUNT);
}

} // end namespace anki
