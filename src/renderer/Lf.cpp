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
	enabled = initializer.get("pps.lf.enabled") 
		&& initializer.get("pps.hdr.enabled");
	if(!enabled)
	{
		return;
	}

	maxFlaresPerLight = initializer.get("pps.lf.maxFlaresPerLight");
	maxLightsWithFlares = initializer.get("pps.lf.maxLightsWithFlares");

	// Load program 1
	std::string pps = "#define TEX_DIMENSIONS vec2(" 
		+ std::to_string(r->getPps().getHdr().getFai().getWidth()) + ".0, "
		+ std::to_string(r->getPps().getHdr().getFai().getHeight()) + ".0)\n";

	pseudoProg.load(ShaderProgramResource::createSrcCodeToCache(
		"shaders/PpsLfPseudoPass.glsl", pps.c_str(), "r_").c_str());

	// Load program 2
	pps = "#define MAX_FLARES "
		+ std::to_string(maxFlaresPerLight * maxLightsWithFlares) + "\n";
	std::string fname = ShaderProgramResource::createSrcCodeToCache(
		"shaders/PpsLfSpritePass.glsl", pps.c_str(), "r_");
	realProg.load(fname.c_str());

	ublock = &realProg->findUniformBlock("flaresBlock");
	ublock->setBinding(0);

	PtrSize blockSize = sizeof(Flare) * maxFlaresPerLight * maxLightsWithFlares;
	if(ublock->getSize() != blockSize)
	{
		throw ANKI_EXCEPTION("Incorrect block size");
	}

	// Init UBO
	flareDataUbo.create(blockSize, nullptr);

	// Create the FAI
	fai.create2dFai(r->getPps().getHdr().getFai().getWidth(), 
		r->getPps().getHdr().getFai().getHeight(), 
		GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE);

	fbo.create();
	fbo.setColorAttachments({&fai});
	if(!fbo.isComplete())
	{
		throw ANKI_EXCEPTION("Fbo not complete");
	}

	// Textures
	lensDirtTex.load("engine_data/lens_dirt.ankitex");
}

//==============================================================================
void Lf::run()
{
	ANKI_ASSERT(enabled);

	//
	// First pass
	//

	// Set the common state
	const Texture& inTex = r->getPps().getHdr().getFai();

	fbo.bind();
	pseudoProg->bind();
	pseudoProg->findUniformVariable("tex").set(inTex);
	pseudoProg->findUniformVariable("lensDirtTex").set(*lensDirtTex);

	GlStateSingleton::get().setViewport(
		0, 0, inTex.getWidth(), inTex.getHeight());
	GlStateSingleton::get().disable(GL_DEPTH_TEST);
	GlStateSingleton::get().disable(GL_BLEND);

	r->drawQuad();

	//
	// Rest of the passes
	//

	// Retrieve some things
	SceneGraph& scene = r->getSceneGraph();
	Camera& cam = scene.getActiveCamera();
	VisibilityTestResults& vi = *cam.getVisibilityTestResults();

	// Iterate the visible light and get those that have lens flare
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

	// Sort the lights using their lens flare texture
	std::sort(lights.begin(), lights.begin() + lightsCount, LightSortFunctor());

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

		if(posClip.x() > posClip.w() || posClip.x() < -posClip.w()
			|| posClip.y() > posClip.w() || posClip.y() < -posClip.w())
		{
			continue;
		}

		Vec2 posNdc = posClip.xy() / posClip.w();

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

		// First flare 
		F32 stretchFactor = 1.0 - posNdc.getLength();
		stretchFactor *= stretchFactor;

		Vec2 stretch = light.getLensFlaresStretchMultiplier() * stretchFactor;

		flares[flaresCount].pos = posNdc;
		flares[flaresCount].scale = 
			light.getLensFlaresSize() * Vec2(1.0, r->getAspectRatio()) 
			* stretch;
		flares[flaresCount].depth = 0.0;
		flares[flaresCount].alpha = light.getLensFlaresAlpha() * stretchFactor;
		++flaresCount;
		++groups[groupsCount - 1];

		// The rest of the flares
		for(U d = 1; d < depth; d++)
		{
			// Write the "flares"
			F32 factor = d / ((F32)depth - 1.0);

			F32 flen = len * 2.0 * factor;

			flares[flaresCount].pos = posNdc + dir * flen;

			flares[flaresCount].scale = 
				light.getLensFlaresSize() * Vec2(1.0, r->getAspectRatio())
				* ((len - flen) * 2.0);

			flares[flaresCount].depth = d;

			flares[flaresCount].alpha = light.getLensFlaresAlpha();

			// Advance
			++flaresCount;
			++groups[groupsCount - 1];
		}
	}

	// Time to render
	//

	// Write the buffer
	flareDataUbo.write(&flares[0], 0, sizeof(Flare) * flaresCount);

	// Set the common state
	realProg->bind();
	GlStateSingleton::get().disable(GL_DEPTH_TEST);
	GlStateSingleton::get().enable(GL_BLEND);
	GlStateSingleton::get().setBlendFunctions(GL_ONE, GL_ONE);

	PtrSize offset = 0;
	for(U i = 0; i < groupsCount; i++)
	{
		const Texture& tex = *texes[i];
		U instances = groups[i];
		PtrSize buffSize = sizeof(Flare) * instances;

		realProg->findUniformVariable("images").set(tex);

		flareDataUbo.setBindingRange(0, offset, buffSize);
		
		r->drawQuadInstanced(instances);

		offset += buffSize;
	}
}

} // end namespace anki
