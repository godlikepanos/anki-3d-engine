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
	U32 lightIndices[Is::MAX_LIGHTS_PER_TILE];
};

struct ShaderTiles
{
	ShaderTile tile[Is::TILES_X_COUNT * Is::TILES_X_COUNT];
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
	std::string pps;

	// point light
	lightSProgs[LST_POINT].load(ShaderProgramResource::createSrcCodeToCache(
		"shaders/IsLpGeneric.glsl", "#define POINT_LIGHT 1\n").c_str());

	// spot light no shadow
	lightSProgs[LST_SPOT].load(
		ShaderProgramResource::createSrcCodeToCache(
		"shaders/IsLpGeneric.glsl", "#define SPOT_LIGHT 1\n").c_str());

	// spot light w/t shadow
	pps = std::string("#define SPOT_LIGHT 1\n"
		"#define SHADOW 1\n");
	if(/*sm.isPcfEnabled()*/ 1) // XXX
	{
		pps += "#define PCF 1\n";
	}
	lightSProgs[LST_SPOT_SHADOW].load(
		ShaderProgramResource::createSrcCodeToCache(
		"shaders/IsLpGeneric.glsl", pps.c_str()).c_str());

	// Min max
	pps =
		"#define TILES_X_COUNT " + std::to_string(TILES_X_COUNT) + "\n"
		"#define TILES_Y_COUNT " + std::to_string(TILES_Y_COUNT) + "\n"
		"#define RENDERER_WIDTH " + std::to_string(r->getWidth()) + "\n"
		"#define RENDERER_HEIGHT " + std::to_string(r->getWidth()) + "\n";
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
	fbo.setOtherAttachment(GL_DEPTH_STENCIL_ATTACHMENT,
		r->getMs().getDepthFai());

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
	F32 tileWidth = 1.0 / TILES_X_COUNT;
	F32 tileHeight = 1.0 / TILES_Y_COUNT;

	for(U i = 0; i < TILES_X_COUNT; i++)
	{
		for(U j = 0; j < TILES_Y_COUNT; j++)
		{
		}
	}
}

//==============================================================================
void Is::projectShape(const Camera& cam,
	const Sphere& sphere, Vec2& circleCenter, F32& circleRadius)
{
	F32 r = sphere.getRadius();
	Vec4 e(cam.getWorldTransform().getOrigin(), 1.0);
	Vec4 c(sphere.getCenter(), 1.0);
	Vec4 a = c + (c - e).getNormalized() * r;

	c = cam.getViewProjectionMatrix() * c;
	c /= c.w();

	a = cam.getViewProjectionMatrix() * a;
	a /= a.w();

	circleCenter = Vec2(c.x(), c.y());
	circleRadius = (c - a).getLength();
}

//==============================================================================
void Is::minMaxPass()
{
	// Do the pass
	//
	const Camera& cam = r->getScene().getActiveCamera();

	GlStateSingleton::get().disable(GL_DEPTH_TEST);

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
	F32 pixels[TILES_X_COUNT][TILES_Y_COUNT][2];

	minMaxFai.readPixels(pixels);

	for(U i = 0; i < TILES_X_COUNT; i++)
	{
		for(U j = 0; j < TILES_Y_COUNT; j++)
		{
			Tile& tile = tiles[i][j];
			tile.depth = Vec2(pixels[i][j][0], pixels[i][j][1]);
		}
	}
}

//==============================================================================
void Is::run()
{
	/*fbo.bind();
	GlStateSingleton::get().setViewport(0, 0, r->getWidth(), r->getHeight());
	const Camera& cam = r->getScene().getActiveCamera();*/

	//minMaxPass();

	fbo.bind();
	glClear(GL_COLOR_BUFFER_BIT);
}

} // end namespace anki
