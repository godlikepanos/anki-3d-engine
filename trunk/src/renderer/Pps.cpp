#include "anki/renderer/Pps.h"
#include "anki/renderer/Renderer.h"
#include "anki/renderer/Hdr.h"
#include "anki/renderer/Ssao.h"
#include "anki/core/Logger.h"

namespace anki {

//==============================================================================
Pps::Pps(Renderer* r_)
	: OptionalRenderingPass(r_), hdr(r_), ssao(r_), bl(r_), lf(r_)
{}

//==============================================================================
Pps::~Pps()
{}

//==============================================================================
void Pps::initInternal(const RendererInitializer& initializer)
{
	enabled = initializer.get("pps.enabled");
	if(!enabled)
	{
		return;
	}

	ANKI_ASSERT("Initializing PPS");

	ssao.init(initializer);
	hdr.init(initializer);
	lf.init(initializer);

	// FBO
	fai.create2dFai(r->getWidth(), r->getHeight(), GL_RGB8, GL_RGB,
		GL_UNSIGNED_BYTE);

	fbo.create({{&fai, GL_COLOR_ATTACHMENT0}});

	// SProg
	std::string pps = "";
	if(ssao.getEnabled())
	{
		pps += "#define SSAO_ENABLED\n";
	}

	if(hdr.getEnabled())
	{
		pps += "#define HDR_ENABLED\n";
	}

	if(lf.getEnabled())
	{
		pps += "#define LF_ENABLED\n";
	}

	if(initializer.get("pps.sharpen"))
	{
		pps += "#define SHARPEN_ENABLED\n";
	}

	if(initializer.get("pps.gammaCorrection"))
	{
		pps += "#define GAMMA_CORRECTION_ENABLED\n";
	}

	pps += "#define FBO_WIDTH " + std::to_string(r->getWidth()) + "\n";
	pps += "#define FBO_HEIGHT " + std::to_string(r->getHeight()) + "\n";

	prog.load(ShaderProgramResource::createSrcCodeToCache(
		"shaders/Pps.glsl", pps.c_str(), "r_").c_str());
}

//==============================================================================
void Pps::init(const RendererInitializer& initializer)
{
	try
	{
		initInternal(initializer);
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to init PPS") << e;
	}
}

//==============================================================================
void Pps::run()
{
	ANKI_ASSERT(enabled);

	GlStateSingleton::get().disable(GL_BLEND);

	// First SSAO because it depends on MS where HDR depends on IS
	if(ssao.getEnabled())
	{
		ssao.run();
	}

	if(hdr.getEnabled())
	{
		hdr.run();
	}

	if(lf.getEnabled())
	{
		lf.run();
	}

	Bool drawToDefaultFbo = 
		!r->getDbg().getEnabled() 
		&& !r->getIsOffscreen()
		&& r->getRenderingQuality() == 1.0;

	if(drawToDefaultFbo)
	{
		Fbo::bindDefault(true);
		GlStateSingleton::get().setViewport(
			0, 0, r->getWindowWidth(), r->getWindowHeight());
	}
	else
	{
		fbo.bind(true);
		GlStateSingleton::get().setViewport(
			0, 0, r->getWidth(), r->getHeight());
	}

	GlStateSingleton::get().enable(GL_DEPTH_TEST, false);
	GlStateSingleton::get().enable(GL_BLEND, false);

	prog->bind();
	prog->findUniformVariable("isFai").set(r->getIs().getFai());

	if(ssao.getEnabled())
	{
		prog->findUniformVariable("ppsSsaoFai").set(ssao.getFai());
	}
	if(hdr.getEnabled())
	{
		prog->findUniformVariable("ppsHdrFai").set(hdr.getFai());
	}
	if(lf.getEnabled())
	{
		prog->findUniformVariable("ppsLfFai").set(lf.getFai());
	}

	r->drawQuad();
}

} // end namespace anki
