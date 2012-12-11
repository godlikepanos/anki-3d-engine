#include "anki/renderer/Pps.h"
#include "anki/renderer/Renderer.h"
#include "anki/renderer/Hdr.h"
#include "anki/renderer/Ssao.h"

namespace anki {

//==============================================================================
Pps::Pps(Renderer* r_)
	: SwitchableRenderingPass(r_), hdr(r_), ssao(r_), bl(r_)
{}

//==============================================================================
Pps::~Pps()
{}

//==============================================================================
void Pps::initInternal(const RendererInitializer& initializer)
{
	enabled = initializer.pps.enabled;
	if(!enabled)
	{
		return;
	}

	ssao.init(initializer);
	hdr.init(initializer);
	drawToDefaultFbo = initializer.pps.drawToDefaultFbo;

	// FBO
	if(!drawToDefaultFbo)
	{
		Renderer::createFai(r->getWidth(), r->getHeight(), GL_RGB, GL_RGB,
			GL_FLOAT, fai);

		fbo.create();
		fbo.setColorAttachments({&fai});
		if(!fbo.isComplete())
		{
			throw ANKI_EXCEPTION("Fbo not complete");
		}
	}
	else
	{
		width = initializer.width / initializer.renderingQuality;
		height = initializer.height / initializer.renderingQuality;
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
