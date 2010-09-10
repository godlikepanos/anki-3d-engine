#include "Pps.h"
#include "Renderer.h"


//======================================================================================================================
// initPassFbo                                                                                                         =
//======================================================================================================================
void Pps::initPassFbo(Fbo& fbo, Texture& fai, const char* msg)
{
	fbo.create();
	fbo.bind();

	fbo.setNumOfColorAttachements(1);

	fai.createEmpty2D(r.getWidth(), r.getHeight(), GL_RGB, GL_RGB, GL_FLOAT);
	fai.setRepeat(false);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fai.getGlId(), 0);

	if(!fbo.isGood())
		FATAL(msg);

	fbo.unbind();
}


//======================================================================================================================
// initPrePassSProg                                                                                                    =
//======================================================================================================================
void Pps::initPrePassSProg()
{
	string pps = "";
	string prefix = "";

	if(ssao.isEnabled())
	{
		pps += "#define SSAO_ENABLED\n";
		prefix += "Ssao";
	}

	prePassSProg.loadRsrc(ShaderProg::createSrcCodeToCache("shaders/PpsPrePass.glsl", pps.c_str(),
	                                                       prefix.c_str()).c_str());
	prePassSProg->bind();

	if(ssao.isEnabled())
	{
		prePassSProgUniVars.ppsSsaoFai = prePassSProg->findUniVar("ppsSsaoFai");
	}

	prePassSProgUniVars.isFai = prePassSProg->findUniVar("isFai");
}


//======================================================================================================================
// initPostPassSProg                                                                                                   =
//======================================================================================================================
void Pps::initPostPassSProg()
{
	string pps = "";
	string prefix = "";

	if(hdr.isEnabled())
	{
		pps += "#define HDR_ENABLED\n";
		prefix += "Hdr";
	}

	postPassSProg.loadRsrc(ShaderProg::createSrcCodeToCache("shaders/PpsPostPass.glsl", pps.c_str(),
	                                                        prefix.c_str()).c_str());
	postPassSProg->bind();

	if(hdr.isEnabled())
		postPassSProgUniVars.ppsHdrFai = postPassSProg->findUniVar("ppsHdrFai");

	postPassSProgUniVars.ppsPrePassFai = postPassSProg->findUniVar("ppsPrePassFai");
}


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Pps::init(const RendererInitializer& initializer)
{
	ssao.init(initializer);
	hdr.init(initializer);

	initPassFbo(prePassFbo, prePassFai, "Cannot create pre-pass post-processing stage FBO");
	initPassFbo(postPassFbo, postPassFai, "Cannot create post-pass post-processing stage FBO");

	initPrePassSProg();
	initPostPassSProg();
}


//======================================================================================================================
// runPrePass                                                                                                          =
//======================================================================================================================
void Pps::runPrePass()
{
	if(ssao.isEnabled())
		ssao.run();

	prePassFbo.bind();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	Renderer::setViewport(0, 0, r.getWidth(), r.getHeight());

	prePassSProg->bind();
	prePassSProgUniVars.isFai->setTexture(r.is.fai, 0);

	if(ssao.isEnabled())
	{
		prePassSProgUniVars.ppsSsaoFai->setTexture(ssao.fai, 1);
	}

	Renderer::drawQuad(0);

	Fbo::unbind();

	prePassFai.genMipmap();
}


//======================================================================================================================
// runPostPass                                                                                                         =
//======================================================================================================================
void Pps::runPostPass()
{
	if(hdr.isEnabled())
		hdr.run();

	postPassFbo.bind();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	Renderer::setViewport(0, 0, r.getWidth(), r.getHeight());

	postPassSProg->bind();
	postPassSProgUniVars.ppsPrePassFai->setTexture(prePassFai, 0);

	if(hdr.isEnabled())
	{
		postPassSProgUniVars.ppsHdrFai->setTexture(hdr.fai, 1);
	}

	Renderer::drawQuad(0);

	Fbo::unbind();
}
