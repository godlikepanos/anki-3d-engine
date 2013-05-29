#include "anki/renderer/Pps.h"
#include "anki/renderer/Renderer.h"
#include "anki/renderer/Hdr.h"
#include "anki/renderer/Ssao.h"
#include "anki/core/Logger.h"

namespace anki {

//==============================================================================
Pps::Pps(Renderer* r_)
	: RenderingPass(r_), hdr(r_), ssao(r_), bl(r_)
{}

//==============================================================================
Pps::~Pps()
{}

//==============================================================================
void Pps::initInternal(const RendererInitializer& initializer)
{
	ssao.init(initializer);
	hdr.init(initializer);

	width = initializer.width / initializer.renderingQuality;
	height = initializer.height / initializer.renderingQuality;

	// FBO
	Renderer::createFai(r->getWidth(), r->getHeight(), GL_RGB, GL_RGB,
		GL_UNSIGNED_BYTE, fai);

	fbo.create();
	fbo.setColorAttachments({&fai});
	if(!fbo.isComplete())
	{
		throw ANKI_EXCEPTION("Fbo not complete");
	}

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

	if(initializer.pps.sharpen)
	{
		pps += "#define SHARPEN_ENABLED\n";
	}

	pps += "#define FBO_WIDTH " + std::to_string(r->getWidth()) + "\n";
	pps += "#define FBO_HEIGHT " + std::to_string(r->getHeight()) + "\n";

	prog.load(ShaderProgramResource::createSrcCodeToCache(
		"shaders/Pps.glsl", pps.c_str()).c_str());
}

//==============================================================================
void Pps::init(const Renderer::Initializer& initializer)
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

	if(drawToDefaultFbo)
	{
		Fbo::bindDefault();
		GlStateSingleton::get().setViewport(
			0, 0, width, height);
	}
	else
	{
		fbo.bind();
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
	//prog->findUniformVariable("msDepthFai").set(r->getMs().getDepthFai());

	r->drawQuad();
}

} // end namespace anki
