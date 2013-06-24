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
	F32 depth; ///< Texture depth
	U32 padding[2];
};

//==============================================================================
struct LightSortFunctor
{
	Bool operator()(const Light* lightA, const Light* lightB)
	{
		ANKI_ASSERT(lightA && lightB);
		ANKI_ASSERT(lightA->hasLensFlare() && lightB->hasLensFlare());

		return lightA->getLensFlareTexture().getGlId() < 
			lightB->getLensFlareTexture().getGlId();
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

	// Retrieve some things
	SceneGraph& scene = r->getSceneGraph();
	Camera& cam = scene.getActiveCamera();
	VisibilityTestResults& vi = *cam.getVisibilityTestResults();

	//
	// Iterate the visible light and get those that have lens flare
	//
	Array<Light*, ANKI_MAX_LIGHTS_WITH_FLARE> lights;
	U lightsCount = 0;
	for(auto it = vi.getLightsBegin(); it != vi.getLightsEnd(); ++it)
	{
		SceneNode& sn = *(*it).node;
		ANKI_ASSERT(sn.getLight());
		Light* light = sn.getLight();

		if(light->hasLensFlare())
		{
			lights[lightsCount % maxLightsWithFlares] = light;
			++lightsCount;
		}
	}

	// Early exit
	if(lightsCount == 0)
	{
		return;
	}

	lightsCount = lightsCount % (maxLightsWithFlares + 1);

	//
	// Sort the lights using their lens flare texture
	//
	std::sort(lights.begin(), lights.begin() + lightsCount, LightSortFunctor());

	//
	// Write the UBO and get the groups
	//

	Array<Flare, ANKI_MAX_FLARES> flares;
	U flaresCount = 0;

	// Contains the number of flares per flare texture
	Array<U, ANKI_MAX_LIGHTS_WITH_FLARE> groups;
	Array<const Texture*, ANKI_MAX_LIGHTS_WITH_FLARE> texes;
	U groupsCount = 0;

	GLuint lastTexId = 0;

	// Iterate all lights and update the flares as well as the groups
	while(lightsCount-- != 0)
	{
		Light& light = *lights[lightsCount];
		const Texture& tex = light.getLensFlareTexture();
		const U depth = tex.getDepth();

		// Transform
		Vec3 posWorld = light.getWorldTransform().getOrigin();
		Vec4 posClip = cam.getViewProjectionMatrix() * Vec4(posWorld, 1.0);
		Vec2 posNdc = (posClip.xyz() / posClip.w()).xy();

		Vec2 dir = -posNdc;
		F32 len = dir.getLength();
		dir /= len; // Normalize dir

		// New group?
		if(lastTexId != tex.getGlId())
		{
			texes[groupsCount] = &tex;
			groups[groupsCount] = 0;
			lastTexId = tex.getGlId();

			++groupsCount;
		}

		for(U d = 0; d < depth; d++)
		{
			// Write the "flares"
			F32 factor = d / ((F32)depth - 1.0);

			flares[flaresCount].pos = 
				posNdc + dir * (len * len * 2.0 * factor);

			flares[flaresCount].scale = 
				Vec2(0.1 * 3.0, r->getAspectRatio() * 0.1) 
				* (factor * len + 1.0);

			flares[flaresCount].depth = d;

			// Advance
			++flaresCount;
			++groups[groupsCount - 1];
		}
	}

	//
	// Time to render
	//

	// Write the buffer
	flareDataUbo.write(&flares[0], 0, sizeof(Flare) * flaresCount);

	// Set the common state
	drawProg->bind();
	GlStateSingleton::get().enable(GL_BLEND);
	GlStateSingleton::get().setBlendFunctions(GL_ONE, GL_ONE);

	PtrSize offset = 0;
	for(U i = 0; i < groupsCount; i++)
	{
		const Texture& tex = *texes[i];
		U instances = groups[i];
		PtrSize buffSize = sizeof(Flare) * instances;

		drawProg->findUniformVariable("images").set(tex);

		flareDataUbo.setBindingRange(0, offset, buffSize);
		
		r->drawQuadInstanced(instances);

		offset += buffSize;
	}
}

} // end namespace anki
