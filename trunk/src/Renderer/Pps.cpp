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
	fai.setRepeat(false);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fai.getGlId(), 0);

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
	string prefix = "";

	if(ssao.enabled)
	{
		pps += "#define SSAO_ENABLED\n";
		prefix += "Ssao";
	}

	prePassSProg.reset(Resource::shaders.load(ShaderProg::createSrcCodeToCache("shaders/PpsPrePass.glsl",
	                                                                           pps.c_str(),
	                                                                           prefix.c_str()).c_str()));
	prePassSProg->bind();

	if(ssao.enabled)
	{
		prePassSProgUniVars.ppsSsaoFai = prePassSProg->findUniVar("ppsSsaoFai");
	}

	prePassSProgUniVars.isFai = prePassSProg->findUniVar("isFai");
}


//======================================================================================================================
// initPostPassSProg                                                                                                   =
//======================================================================================================================
void Renderer::Pps::initPostPassSProg()
{
	string pps = "";
	string prefix = "";

	if(hdr.enabled)
	{
		pps += "#define HDR_ENABLED\n";
		prefix += "Hdr";
	}

	postPassSProg.reset(Resource::shaders.load(ShaderProg::createSrcCodeToCache("shaders/PpsPostPass.glsl",
	                                                                            pps.c_str(),
	                                                                            prefix.c_str()).c_str()));
	postPassSProg->bind();

	if(hdr.enabled)
		postPassSProgUniVars.ppsHdrFai = postPassSProg->findUniVar("ppsHdrFai");

	postPassSProgUniVars.ppsPrePassFai = postPassSProg->findUniVar("ppsPrePassFai");
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

	prePassSProg->bind();
	prePassSProgUniVars.isFai->setTexture(r.is.fai, 0);

	if(ssao.enabled)
	{
		prePassSProgUniVars.ppsSsaoFai->setTexture(ssao.fai, 1);
	}

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

	postPassSProg->bind();
	postPassSProgUniVars.ppsPrePassFai->setTexture(prePassFai, 0);

	if(hdr.enabled)
	{
		postPassSProgUniVars.ppsHdrFai->setTexture(hdr.fai, 1);
	}

	Renderer::drawQuad(0);

	Fbo::unbind();
}
