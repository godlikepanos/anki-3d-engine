#include "anki/renderer/Lf.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/Movable.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Light.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

// Some constants
#define ANKI_MAX_LIGHTS_WITH_FLARE 16
#define ANKI_MAX_FLARES_PER_LIGHT 8
#define ANKI_MAX_FLARES (ANKI_MAX_LIGHTS_WITH_FLARE * ANKI_MAX_FLARES_PER_LIGHT)

//==============================================================================
struct Flare
{
	Vec2 pos; ///< Position in NDC
	Vec2 scale; ///< Scale of the quad
	F32 alpha; ///< Alpha value
	U32 padding[3];
};

//==============================================================================
struct LightSortFunctor
{
	Bool operator()(const SceneNode* lightA, const SceneNode* lightB)
	{
		ANKI_ASSERT(lightA && lightB);
		ANKI_ASSERT(lightA->getLight() && lightB->getLight());
		ANKI_ASSERT(lightA->getLight()->hasLensFlare() 
			&& lightB->getLight()->hasLensFlare());

		return lightA->getLight()->getLensFlareTexture().getGlId() < 
			lightB->getLight()->getLensFlareTexture().getGlId();
	}
};

//==============================================================================
// Lf                                                                          =
//==============================================================================

//==============================================================================
Lf::~Lf()
{}

//==============================================================================
void Lf::init(const RendererInitializer& initializer)
{
	try
	{
		initInternal(initializer);
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to init LF") << e;
	}
}

//==============================================================================
void Lf::initInternal(const RendererInitializer& initializer)
{
	enabled = initializer.pps.lf.enabled;
	if(!enabled)
	{
		return;
	}

	maxFlaresPerLight = initializer.pps.lf.maxFlaresPerLight;
	maxLightsWithFlares = initializer.pps.lf.maxLightsWithFlares;

	if(maxLightsWithFlares > ANKI_MAX_LIGHTS_WITH_FLARE)
	{
		throw ANKI_EXCEPTION("Too big maxLightsWithFlares");
	}

	if(maxFlaresPerLight > ANKI_MAX_FLARES_PER_LIGHT)
	{
		throw ANKI_EXCEPTION("Too big maxFlaresPerLight");
	}

	// Load the shader
	std::string pps = "#define MAX_FLARES " 
		+ std::to_string(maxFlaresPerLight * maxLightsWithFlares) + "\n";
	std::string fname = ShaderProgramResource::createSrcCodeToCache(
		"shaders/PpsLfPass.glsl", pps.c_str());
	drawProg.load(fname.c_str());

	ublock = &drawProg->findUniformBlock("flaresBlock");
	ublock->setBinding(0);

	PtrSize blockSize = sizeof(Flare) * maxFlaresPerLight * maxLightsWithFlares;
	if(ublock->getSize() != blockSize)
	{
		throw ANKI_EXCEPTION("Incorrect block size");
	}

	// Init UBO
	flareDataUbo.create(blockSize, nullptr);
}

//==============================================================================
void Lf::run()
{
	ANKI_ASSERT(enabled);

	// Retrive some things
	SceneGraph& scene = r->getSceneGraph();
	Camera& cam = scene.getActiveCamera();
	VisibilityTestResults& vi = *cam.getVisibilityTestResults();

	//
	// Iterate the visible light and get those that have lens flare
	//
	Array<SceneNode*, ANKI_MAX_LIGHTS_WITH_FLARE> lights;

	U lightCount = 0;
	for(auto it = vi.getLightsBegin(); it != vi.getLightsEnd(); ++it)
	{
		SceneNode& sn = *(*it).node;
		ANKI_ASSERT(sn.getLight());
		Light& light = *sn.getLight();

		if(light.hasLensFlare())
		{
			lights[lightCount % maxLightsWithFlares] = &light;
			++lightCount;
		}
	}

	lightCount = lightCount % (maxLightsWithFlares + 1);

	//
	// Sort the lights using the lens flare texture
	//
	std::sort(lights.begin(), lights.begin() + lightCount, LightSortFunctor());

	//
	// Write the UBO and get the groups
	//

	Array<Flare, ANKI_MAX_FLARES> flareBuff;
	Array<U, ANKI_MAX_LIGHTS_WITH_FLARE> groups;
	U groupsCount = 0;

	while(lightCount-- != 0)
	{
		Light& light = *lights[lightCount]->getLight();
	}

#if 0
	for(auto it = vi.getLightsBegin(); it != vi.getLightsEnd(); ++it)
	{
		SceneNode& sn = *(*it).node;
		ANKI_ASSERT(sn.getLight());
		Light& light = *sn.getLight();

		if(!light.hasLensFlare())
		{
			continue;
		}

		// Transform
		Vec3 posworld = sn.getMovable()->getWorldTransform().getOrigin();
		Vec4 posclip = cam.getViewProjectionMatrix() * Vec4(posworld, 1.0);
		Vec2 posndc = (posclip.xyz() / posclip.w()).xy();

		Vec2 dir = -posndc;
		F32 len = dir.getLength() * 2.0;
		dir /= len;

		const Texture& tex = light.getLensFlareTexture();
		const U depth = tex.getDepth();

		for(U d = 0; d < depth; d++)
		{
			flareBuff[count].pos = posndc + dir * (len * (d / (F32)depth));
			flareBuff[count].scale = Vec2(0.1 * 3.0, r->getAspectRatio() * 0.1);
			++count;
		}
	}

	flareDataUbo.write(&flareBuff[0], 0, sizeof(Flare) * count);

	// Draw
	drawProg->bind();
	drawProg->findUniformVariable("images").set(*tex);
	flareDataUbo.setBinding(0);

	GlStateSingleton::get().enable(GL_BLEND);
	GlStateSingleton::get().setBlendFunctions(GL_ONE, GL_ONE);
	r->drawQuadInstanced(tex->getDepth());

#endif
}

} // end namespace anki
