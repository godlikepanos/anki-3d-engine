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

	// Load the shader
	std::string pps = "#define TEX_DIMENSIONS vec2(" 
		+ std::to_string(r->getPps().getHdr().getFai().getWidth()) + ".0, "
		+ std::to_string(r->getPps().getHdr().getFai().getHeight()) + ".0)\n";

	drawProg.load(ShaderProgramResource::createSrcCodeToCache(
		"shaders/PpsLfPass.glsl", pps.c_str()).c_str());

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

	// Set the common state
	const Texture& inTex = r->getPps().getHdr().getFai();

	fbo.bind();
	drawProg->bind();
	drawProg->findUniformVariable("tex").set(inTex);
	drawProg->findUniformVariable("lensDirtTex").set(*lensDirtTex);

	GlStateSingleton::get().setViewport(
		0, 0, inTex.getWidth(), inTex.getHeight());
	GlStateSingleton::get().disable(GL_DEPTH_TEST);
	GlStateSingleton::get().disable(GL_BLEND);

	r->drawQuad();
}

} // end namespace anki
