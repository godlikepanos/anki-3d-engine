#include <boost/lexical_cast.hpp>
#include "Hdr.h"
#include "Renderer.h"
#include "RendererInitializer.h"


//======================================================================================================================
// initFbo                                                                                                            =
//======================================================================================================================
void Hdr::initFbo(Fbo& fbo, Texture& fai)
{
	int width = renderingQuality * r.getWidth();
	int height = renderingQuality * r.getHeight();

	// create FBO
	fbo.create();
	fbo.bind();

	// inform in what buffers we draw
	fbo.setNumOfColorAttachements(1);

	// create the texes
	fai.createEmpty2D(width, height, GL_RGB, GL_RGB, GL_FLOAT);
	fai.setTexParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	fai.setTexParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// attach
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fai.getGlId(), 0);

	// test if success
	if(!fbo.isGood())
		FATAL("Cannot create deferred shading post-processing stage HDR passes FBO");

	// unbind
	fbo.unbind();
}


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Hdr::init(const RendererInitializer& initializer)
{
	enabled = initializer.pps.hdr.enabled;

	if(!enabled)
		return;

	renderingQuality = initializer.pps.hdr.renderingQuality;
	blurringDist = initializer.pps.hdr.blurringDist;
	blurringIterations = initializer.pps.hdr.blurringIterations;

	initFbo(toneFbo, toneFai);
	initFbo(hblurFbo, hblurFai);
	initFbo(vblurFbo, fai);

	fai.setTexParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	// init shaders
	toneSProg.loadRsrc("shaders/PpsHdr.glsl");
	toneProgFaiUniVar = toneSProg->findUniVar("fai");

	const char* SHADER_FILENAME = "shaders/GaussianBlurGeneric.glsl";

	string pps = "#define HPASS\n#define COL_RGB\n";
	string prefix = "HorizontalRgb";
	hblurSProg.loadRsrc(ShaderProg::createSrcCodeToCache(SHADER_FILENAME, pps.c_str(), prefix.c_str()).c_str());
	hblurSProgFaiUniVar = hblurSProg->findUniVar("img");

	pps = "#define VPASS\n#define COL_RGB\n";
	prefix = "VerticalRgb";
	vblurSProg.loadRsrc(ShaderProg::createSrcCodeToCache(SHADER_FILENAME, pps.c_str(), prefix.c_str()).c_str());
	vblurSProgFaiUniVar = vblurSProg->findUniVar("img");
}

#include "App.h"

//======================================================================================================================
// runPass                                                                                                             =
//======================================================================================================================
void Hdr::run()
{
	int w = renderingQuality * r.getWidth();
	int h = renderingQuality * r.getHeight();
	Renderer::setViewport(0, 0, w, h);

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	// pass 0
	toneFbo.bind();
	toneSProg->bind();
	toneProgFaiUniVar->setTexture(r.pps.prePassFai, 0);
	Renderer::drawQuad(0);


	// blurring passes
	hblurFai.setRepeat(false);
	fai.setRepeat(false);
	for(uint i=0; i<blurringIterations; i++)
	{
		// hpass
		hblurFbo.bind();
		hblurSProg->bind();
		if(i == 0)
		{
			hblurSProgFaiUniVar->setTexture(toneFai, 0);
		}
		else
		{
			hblurSProgFaiUniVar->setTexture(fai, 0);
		}
		hblurSProg->findUniVar("imgDimension")->setFloat(w);
		hblurSProg->findUniVar("blurringDist")->setFloat(blurringDist / w);
		Renderer::drawQuad(0);

		// vpass
		vblurFbo.bind();
		vblurSProg->bind();
		vblurSProgFaiUniVar->setTexture(hblurFai, 0);
		vblurSProg->findUniVar("imgDimension")->setFloat(h);
		vblurSProg->findUniVar("blurringDist")->setFloat(blurringDist / h);
		Renderer::drawQuad(0);
	}

	// end
	Fbo::unbind();
}
