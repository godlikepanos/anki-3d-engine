/**
 * @file
 *
 * Post-processing stage
 */

#include "Renderer.h"


//======================================================================================================================
// initPassFbo                                                                                                         =
//======================================================================================================================
void Renderer::Pps::initPassFbo(Fbo& fbo, Texture& fai, const char* msg)
{
	fbo.create();
	fbo.bind();

	fbo.setNumOfColorAttachements(1);

	fai.createEmpty2D(r.width, r.height, GL_RGB, GL_RGB, GL_FLOAT, false);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fai.getGlId(), 0);

	if(!fbo.isGood())
		FATAL(msg);

	fbo.unbind();
}


//======================================================================================================================
// initPrePassSProg                                                                                                    =
//======================================================================================================================
void Renderer::Pps::initPrePassSProg()
{
	string pps = "";
	if(ssao.enabled)
		pps += "#define SSAO_ENABLED\n";

	prePassSProg.customLoad("shaders/PpsPrePass.glsl", pps.c_str());
	prePassSProg.bind();

	if(ssao.enabled)
	{
		prePassSProg.uniVars.ppsSsaoFai = prePassSProg.findUniVar("ppsSsaoFai");
	}

	prePassSProg.uniVars.isFai = prePassSProg.findUniVar("isFai");
}


//======================================================================================================================
// initPostPassSProg                                                                                                   =
//======================================================================================================================
void Renderer::Pps::initPostPassSProg()
{
	string pps = "";
	if(hdr.enabled)
		pps += "#define HDR_ENABLED\n";

	postPassSProg.customLoad("shaders/PpsPostPass.glsl", pps.c_str());
	postPassSProg.bind();

	if(hdr.enabled)
		postPassSProg.uniVars.ppsHdrFai = postPassSProg.findUniVar("ppsHdrFai");

	postPassSProg.uniVars.ppsPrePassFai = postPassSProg.findUniVar("ppsPrePassFai");
}


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Renderer::Pps::init()
{
	if(ssao.enabled)
		ssao.init();

	if(hdr.enabled)
		hdr.init();

	initPassFbo(prePassFbo, prePassFai, "Cannot create pre-pass post-processing stage FBO");
	initPassFbo(postPassFbo, postPassFai, "Cannot create post-pass post-processing stage FBO");

	initPrePassSProg();
	initPostPassSProg();
}


//======================================================================================================================
// runPrePass                                                                                                          =
//======================================================================================================================
void Renderer::Pps::runPrePass()
{
	if(ssao.enabled)
		ssao.run();

	prePassFbo.bind();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	Renderer::setViewport(0, 0, r.width, r.height);

	prePassSProg.bind();
	prePassSProg.uniVars.isFai->setTexture(r.is.fai, 0);

	if(ssao.enabled)
		prePassSProg.uniVars.ppsSsaoFai->setTexture(ssao.fai, 1);

	Renderer::drawQuad(0);

	Fbo::unbind();
}


//======================================================================================================================
// runPostPass                                                                                                         =
//======================================================================================================================
void Renderer::Pps::runPostPass()
{
	if(hdr.enabled)
		hdr.run();

	postPassFbo.bind();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	Renderer::setViewport(0, 0, r.width, r.height);

	postPassSProg.bind();
	postPassSProg.uniVars.ppsPrePassFai->setTexture(prePassFai, 0);

	if(hdr.enabled)
		postPassSProg.uniVars.ppsHdrFai->setTexture(hdr.fai, 1);

	Renderer::drawQuad(0);

	Fbo::unbind();
}
