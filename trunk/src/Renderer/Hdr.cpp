#include <boost/lexical_cast.hpp>
#include "Hdr.h"
#include "Renderer.h"
#include "RendererInitializer.h"


namespace R {


//==============================================================================
// initFbo                                                                    =
//==============================================================================
void Hdr::initFbo(Fbo& fbo, Texture& fai)
{
	try
	{
		int width = renderingQuality * r.getWidth();
		int height = renderingQuality * r.getHeight();

		// create FBO
		fbo.create();
		fbo.bind();

		// inform in what buffers we draw
		fbo.setNumOfColorAttachements(1);

		// create the FAI
		Renderer::createFai(width, height, GL_RGB, GL_RGB, GL_FLOAT, fai);

		// attach
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, fai.getGlId(), 0);

		// test if success
		fbo.checkIfGood();

		// unbind
		fbo.unbind();
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Cannot create deferred shading post-processing "
			"stage HDR passes FBO: " + e.what());
	}
}


//==============================================================================
// init                                                                        =
//==============================================================================
void Hdr::init(const RendererInitializer& initializer)
{
	enabled = initializer.pps.hdr.enabled;

	if(!enabled)
	{
		return;
	}

	renderingQuality = initializer.pps.hdr.renderingQuality;
	blurringDist = initializer.pps.hdr.blurringDist;
	blurringIterationsNum = initializer.pps.hdr.blurringIterationsNum;
	exposure = initializer.pps.hdr.exposure;

	initFbo(toneFbo, toneFai);
	initFbo(hblurFbo, hblurFai);
	initFbo(vblurFbo, fai);

	fai.setFiltering(Texture::TFT_LINEAR);


	// init shaders
	toneSProg.loadRsrc("shaders/PpsHdr.glsl");

	const char* SHADER_FILENAME = "shaders/GaussianBlurGeneric.glsl";

	std::string pps = "#define HPASS\n#define COL_RGB\n";
	hblurSProg.loadRsrc(ShaderProg::createSrcCodeToCache(SHADER_FILENAME,
		pps.c_str()).c_str());

	pps = "#define VPASS\n#define COL_RGB\n";
	vblurSProg.loadRsrc(ShaderProg::createSrcCodeToCache(SHADER_FILENAME,
		pps.c_str()).c_str());
}


//==============================================================================
// runPass                                                                     =
//==============================================================================
void Hdr::run()
{
	/*if(r.getFramesNum() % 2 == 0)
	{
		return;
	}*/

	int w = renderingQuality * r.getWidth();
	int h = renderingQuality * r.getHeight();
	GlStateMachineSingleton::getInstance().setViewport(0, 0, w, h);

	GlStateMachineSingleton::getInstance().enable(GL_BLEND, false);
	GlStateMachineSingleton::getInstance().enable(GL_DEPTH_TEST, false);

	// pass 0
	toneFbo.bind();
	toneSProg->bind();
	toneSProg->getUniformVariable("exposure").set(&exposure);
	toneSProg->getUniformVariable("fai").set(r.getPps().getPrePassFai(), 0);
	r.drawQuad();

	// blurring passes
	hblurFai.setRepeat(false);
	fai.setRepeat(false);
	for(uint i=0; i<blurringIterationsNum; i++)
	{
		// hpass
		hblurFbo.bind();
		hblurSProg->bind();
		if(i == 0)
		{
			hblurSProg->getUniformVariable("img").set(toneFai, 0);
		}
		else
		{
			hblurSProg->getUniformVariable("img").set(fai, 0);
		}
		float tmp = float(w);
		hblurSProg->getUniformVariable("imgDimension").set(&tmp);
		tmp = blurringDist / w;
		hblurSProg->getUniformVariable("blurringDist").set(&tmp);
		r.drawQuad();

		// vpass
		vblurFbo.bind();
		vblurSProg->bind();
		vblurSProg->getUniformVariable("img").set(hblurFai, 0);
		tmp = float(h);
		vblurSProg->getUniformVariable("imgDimension").set(&tmp);
		tmp = blurringDist / h;
		vblurSProg->getUniformVariable("blurringDist").set(&tmp);
		r.drawQuad();
	}

	// end
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind the window framebuffer
}


} // end namespace
