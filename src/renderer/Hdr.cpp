#include <boost/lexical_cast.hpp>
#include "anki/renderer/Hdr.h"
#include "anki/renderer/Renderer.h"

namespace anki {

//==============================================================================
Hdr::~Hdr()
{}

//==============================================================================
void Hdr::initFbo(Fbo& fbo, Texture& fai)
{
	try
	{
		Renderer::createFai(width, height, GL_RGB8, GL_RGB, GL_FLOAT, fai);

		// create FBO
		fbo.create();
		fbo.setColorAttachments({&fai});
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Cannot create deferred shading "
			"post-processing stage HDR passes FBO") << e;
	}
}

//==============================================================================
void Hdr::init(const Renderer::Initializer& initializer)
{
	enabled = initializer.pps.hdr.enabled;

	if(!enabled)
	{
		return;
	}

	renderingQuality = initializer.pps.hdr.renderingQuality;

	width = renderingQuality * r->getWidth();
	height = renderingQuality * r->getHeight();

	initFbo(toneFbo, toneFai);
	initFbo(hblurFbo, hblurFai);
	initFbo(vblurFbo, fai);

	fai.setFiltering(Texture::TFT_LINEAR);

	// init shaders
	toneSProg.load("shaders/PpsHdr.glsl");

	const char* SHADER_FILENAME = "shaders/GaussianBlurGeneric.glsl";

	std::string pps = "#define HPASS\n#define COL_RGB\n#define IMG_DIMENTION "
		+ std::to_string(width) + ".0\n";
	hblurSProg.load(SHADER_FILENAME, pps.c_str());

	pps = "#define VPASS\n#define COL_RGB\n#define IMG_DIMENTION "
		+ std::to_string(height) + ".0\n";
	vblurSProg.load(SHADER_FILENAME, pps.c_str());

	// Set the uniforms
	setBlurringDistance(initializer.pps.hdr.blurringDist);
	blurringIterationsNum = initializer.pps.hdr.blurringIterationsNum;
	setExposure(initializer.pps.hdr.exposure);
}

//==============================================================================
void Hdr::run()
{
	/*if(r.getFramesNum() % 2 == 0)
	{
		return;
	}*/

	GlStateSingleton::get().setViewport(0, 0, width, height);

	GlStateSingleton::get().disable(GL_BLEND);
	GlStateSingleton::get().disable(GL_DEPTH_TEST);

	// pass 0
	toneFbo.bind();
	toneSProg.bind();
	toneSProg.findUniformVariableByName("fai")->set(
		r->getPps().getPrePassFai());
	r->drawQuad();

	// blurring passes
	for(uint32_t i = 0; i < blurringIterationsNum; i++)
	{
		// hpass
		hblurFbo.bind();
		hblurSProg.bind();
		if(i == 0)
		{
			hblurSProg.findUniformVariableByName("img")->set(toneFai);
		}
		else if(i == 1)
		{
			hblurSProg.findUniformVariableByName("img")->set(fai);
		}
		r->drawQuad();

		// vpass
		vblurFbo.bind();
		vblurSProg.bind();
		vblurSProg.findUniformVariableByName("img")->set(hblurFai);
		r->drawQuad();
	}
}

} // end namespace
