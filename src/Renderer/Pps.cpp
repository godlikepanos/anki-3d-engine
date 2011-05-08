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


	hBlurSProg.loadRsrc(ShaderProg::createSrcCodeToCache("shaders/PpsBlurGeneric.glsl", "#define HPASS\n",
	                                                     "h").c_str());
	vBlurSProg.loadRsrc(ShaderProg::createSrcCodeToCache("shaders/PpsBlurGeneric.glsl", "#define VPASS\n",
	                                                     "v").c_str());

	Renderer::createFai(r.getWidth(), r.getHeight(), GL_RGB, GL_RGB, GL_FLOAT, blurFai);

	try
	{
		hBlurFbo.create();
		hBlurFbo.bind();
		hBlurFbo.setNumOfColorAttachements(1);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurFai.getGlId(), 0);
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Cannot create h blur post-processing stage FBO: " + e.what());
	}

	try
	{
		vBlurFbo.create();
		vBlurFbo.bind();
		vBlurFbo.setNumOfColorAttachements(1);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postPassFai.getGlId(), 0);
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Cannot create v blur post-processing stage FBO: " + e.what());
	}


	try
	{
		sideBlurFbo.create();
		sideBlurFbo.bind();
		sideBlurFbo.setNumOfColorAttachements(1);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, r.getMs().getNormalFai().getGlId(), 0);
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Cannot create side blur post-processing stage FBO: " + e.what());
	}

	sideBlur.loadRsrc("engine-rsrc/side-blur.png");

	sideBlurSProg.loadRsrc("shaders/PpsSideBlur.glsl");
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

	//Fbo::unbind();
}


//======================================================================================================================
// runBlur                                                                                                             =
//======================================================================================================================
void Pps::runBlur()
{
	GlStateMachineSingleton::getInstance().setBlendingEnabled(false);
	uint blurringIterationsNum = 1;
	for(uint i = 0; i < blurringIterationsNum; i++)
	{
		// hpass
		hBlurFbo.bind();

		hBlurSProg->bind();
		hBlurSProg->findUniVar("img")->set(postPassFai, 0);
		hBlurSProg->findUniVar("msNormalFai")->set(r.getMs().getNormalFai(), 1);
		float tmp = r.getWidth();
		hBlurSProg->findUniVar("imgDimension")->set(&tmp);

		r.drawQuad();

		// vpass
		vBlurFbo.bind();

		vBlurSProg->bind();
		vBlurSProg->findUniVar("img")->set(blurFai, 0);
		vBlurSProg->findUniVar("msNormalFai")->set(r.getMs().getNormalFai(), 1);
		tmp = r.getHeight();
		vBlurSProg->findUniVar("imgDimension")->set(&tmp);

		r.drawQuad();
	}
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

	//
	// todo
	//
	sideBlurFbo.bind();
	sideBlurSProg->bind();
	sideBlurSProg->findUniVar("tex")->set(*sideBlur, 0);
	GlStateMachineSingleton::getInstance().setBlendingEnabled(true);
	glBlendFunc(GL_ONE, GL_ONE);
	r.drawQuad();


	runBlur();

}
