#include "Pps.h"
#include "Renderer.h"
#include "Hdr.h"
#include "Ssao.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Pps::Pps(Renderer& r_):
	RenderingPass(r_),
	hdr(r_),
	ssao(r_)
{}


//======================================================================================================================
// initPassFbo                                                                                                         =
//======================================================================================================================
void Pps::initPassFbo(Fbo& fbo, Texture& fai)
{
	fbo.create();
	fbo.bind();

	fbo.setNumOfColorAttachements(1);

	Renderer::createFai(r.getWidth(), r.getHeight(), GL_RGB, GL_RGB, GL_FLOAT, fai);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fai.getGlId(), 0);

	fbo.checkIfGood();

	fbo.unbind();
}


//======================================================================================================================
// initPrePassSProg                                                                                                    =
//======================================================================================================================
void Pps::initPrePassSProg()
{
	std::string pps = "";
	std::string prefix = "";

	if(ssao.isEnabled())
	{
		pps += "#define SSAO_ENABLED\n";
		prefix += "Ssao";
	}

	prePassSProg.loadRsrc(ShaderProg::createSrcCodeToCache("shaders/PpsPrePass.glsl", pps.c_str(),
	                                                       prefix.c_str()).c_str());
	prePassSProg->bind();
}


//======================================================================================================================
// initPostPassSProg                                                                                                   =
//======================================================================================================================
void Pps::initPostPassSProg()
{
	std::string pps = "";
	std::string prefix = "";

	if(hdr.isEnabled())
	{
		pps += "#define HDR_ENABLED\n";
		prefix += "Hdr";
	}

	postPassSProg.loadRsrc(ShaderProg::createSrcCodeToCache("shaders/PpsPostPass.glsl", pps.c_str(),
	                                                        prefix.c_str()).c_str());
	postPassSProg->bind();
}


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Pps::init(const RendererInitializer& initializer)
{
	ssao.init(initializer);
	hdr.init(initializer);

	try
	{
		initPassFbo(prePassFbo, prePassFai);
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Cannot create pre-pass post-processing stage FBO: " + e.what());
	}

	try
	{
		initPassFbo(postPassFbo, postPassFai);
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Cannot create post-pass post-processing stage FBO: " + e.what());
	}

	initPrePassSProg();
	initPostPassSProg();
}


//======================================================================================================================
// runPrePass                                                                                                          =
//======================================================================================================================
void Pps::runPrePass()
{
	if(ssao.isEnabled())
	{
		ssao.run();
	}

	prePassFbo.bind();

	GlStateMachineSingleton::getInstance().setDepthTestEnabled(false);
	GlStateMachineSingleton::getInstance().setBlendingEnabled(false);
	Renderer::setViewport(0, 0, r.getWidth(), r.getHeight());

	prePassSProg->bind();
	prePassSProg->findUniVar("isFai")->set(r.getIs().getFai(), 0);

	if(ssao.isEnabled())
	{
		prePassSProg->findUniVar("ppsSsaoFai")->set(ssao.getFai(), 1);
	}

	r.drawQuad();

	Fbo::unbind();
}


//======================================================================================================================
// runPostPass                                                                                                         =
//======================================================================================================================
void Pps::runPostPass()
{
	if(hdr.isEnabled())
	{
		hdr.run();
	}

	postPassFbo.bind();

	GlStateMachineSingleton::getInstance().setDepthTestEnabled(false);
	GlStateMachineSingleton::getInstance().setBlendingEnabled(false);
	Renderer::setViewport(0, 0, r.getWidth(), r.getHeight());

	postPassSProg->bind();
	postPassSProg->findUniVar("ppsPrePassFai")->set(prePassFai, 0);

	if(hdr.isEnabled())
	{
		postPassSProg->findUniVar("ppsHdrFai")->set(hdr.getFai(), 1);
	}

	r.drawQuad();

	Fbo::unbind();
}
