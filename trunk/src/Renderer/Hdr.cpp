/**
 * @file
 *
 * Post-processing stage high dynamic range lighting pass
 */

#include <boost/lexical_cast.hpp>
#include "Renderer.h"


//======================================================================================================================
// initFbos                                                                                                            =
//======================================================================================================================
void Renderer::Pps::Hdr::initFbos(Fbo& fbo, Texture& fai, int internalFormat)
{
	int width = renderingQuality * r.width;
	int height = renderingQuality * r.height;

	// create FBO
	fbo.create();
	fbo.bind();

	// inform in what buffers we draw
	fbo.setNumOfColorAttachements(1);

	// create the texes
	fai.createEmpty2D(width, height, internalFormat, GL_RGB, GL_FLOAT);
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
void Renderer::Pps::Hdr::init()
{
	//int width = renderingQuality * r.width;
	//int height = renderingQuality * r.height;

	initFbos(pass0Fbo, pass0Fai, GL_RGB);
	initFbos(pass1Fbo, pass1Fai, GL_RGB);
	initFbos(pass2Fbo, fai, GL_RGB);

	fai.setTexParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	// init shaders
	pass0SProg.loadRsrc("shaders/PpsHdr.glsl");
	pass0SProgFaiUniVar = pass0SProg->findUniVar("fai");

	const char* shaderFname = "shaders/GaussianBlurGeneric.glsl";

	string pps = "#define HPASS\n#define COL_RGB";
	string prefix = "HRGB";
	pass1SProg.loadRsrc(ShaderProg::createSrcCodeToCache(shaderFname, pps.c_str(), prefix.c_str()).c_str());
	pass1SProgFaiUniVar = pass1SProg->findUniVar("img");

	pps = "#define VPASS\n#define COL_RGB";
	prefix = "VRGB";
	pass1SProg.loadRsrc(ShaderProg::createSrcCodeToCache(shaderFname, pps.c_str(), prefix.c_str()).c_str());
	pass1SProgFaiUniVar = pass1SProg->findUniVar("img");
}


//======================================================================================================================
// runPass                                                                                                             =
//======================================================================================================================
void Renderer::Pps::Hdr::run()
{
	int w = renderingQuality * r.width;
	int h = renderingQuality * r.height;
	Renderer::setViewport(0, 0, w, h);

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	// pass 0
	pass0Fbo.bind();
	pass0SProg->bind();
	r.is.fai.setRepeat(false);
	pass0SProgFaiUniVar->setTexture(r.pps.prePassFai, 0);
	Renderer::drawQuad(0);

	// pass 1
	pass1Fbo.bind();
	pass1SProg->bind();
	pass0Fai.setRepeat(false);
	pass1SProgFaiUniVar->setTexture(pass0Fai, 0);
	Renderer::drawQuad(0);

	// pass 2
	pass2Fbo.bind();
	pass2SProg->bind();
	pass1Fai.setRepeat(false);
	pass2SProgFaiUniVar->setTexture(pass1Fai, 0);
	Renderer::drawQuad(0);

	// end
	Fbo::unbind();
}
