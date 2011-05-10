#include "Pps.h"
#include "Renderer.h"
#include "Hdr.h"
#include "Ssao.h"
#include "RendererInitializer.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Pps::Pps(Renderer& r_):
	RenderingPass(r_),
	hdr(r_),
	ssao(r_)
{}


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Pps::init(const RendererInitializer& initializer)
{
	ssao.init(initializer);
	hdr.init(initializer);
	blurringEnabled = initializer.pps.blurringEnabled;
	blurringIterationsNum = initializer.pps.blurringIterationsNum;

	//
	// Init pre pass
	//

	// FBO
	try
	{
		prePassFbo.create();
		prePassFbo.bind();
		prePassFbo.setNumOfColorAttachements(1);
		Renderer::createFai(r.getWidth(), r.getHeight(), GL_RGB, GL_RGB, GL_FLOAT, prePassFai);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, prePassFai.getGlId(), 0);
		prePassFbo.checkIfGood();
		prePassFbo.unbind();
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Cannot create pre-pass post-processing stage FBO: " + e.what());
	}

	// SProg
	std::string pps = "";
	std::string prefix = "";
	if(ssao.isEnabled())
	{
		pps += "#define SSAO_ENABLED\n";
		prefix += "Ssao";
	}

	prePassSProg.loadRsrc(ShaderProg::createSrcCodeToCache("shaders/PpsPrePass.glsl", pps.c_str(),
	                                                       prefix.c_str()).c_str());

	//
	// Init post pass
	//

	// FBO
	try
	{
		postPassFbo.create();
		postPassFbo.bind();
		postPassFbo.setNumOfColorAttachements(1);
		Renderer::createFai(r.getWidth(), r.getHeight(), GL_RGB, GL_RGB, GL_FLOAT, postPassFai);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postPassFai.getGlId(), 0);
		postPassFbo.checkIfGood();
		postPassFbo.unbind();
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Cannot create post-pass post-processing stage FBO: " + e.what());
	}

	// SProg
	pps = "";
	prefix = "";

	if(hdr.isEnabled())
	{
		pps += "#define HDR_ENABLED\n";
		prefix += "Hdr";
	}

	postPassSProg.loadRsrc(ShaderProg::createSrcCodeToCache("shaders/PpsPostPass.glsl", pps.c_str(),
	                                                        prefix.c_str()).c_str());


	//
	// Blurring
	//
	if(blurringEnabled)
	{
		Renderer::createFai(r.getWidth(), r.getHeight(), GL_RGB, GL_RGB, GL_FLOAT, blurFai);

		// Horizontal
		try
		{
			hBlurFbo.create();
			hBlurFbo.bind();
			hBlurFbo.setNumOfColorAttachements(1);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurFai.getGlId(), 0);
		}
		catch(std::exception& e)
		{
			throw EXCEPTION("Cannot create horizontal blur post-processing stage FBO: " + e.what());
		}

		hBlurSProg.loadRsrc(ShaderProg::createSrcCodeToCache("shaders/PpsBlurGeneric.glsl", "#define HPASS\n",
															 "h").c_str());

		// Vertical
		try
		{
			vBlurFbo.create();
			vBlurFbo.bind();
			vBlurFbo.setNumOfColorAttachements(1);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postPassFai.getGlId(), 0);
		}
		catch(std::exception& e)
		{
			throw EXCEPTION("Cannot create vertical blur post-processing stage FBO: " + e.what());
		}

		vBlurSProg.loadRsrc(ShaderProg::createSrcCodeToCache("shaders/PpsBlurGeneric.glsl", "#define VPASS\n",
															 "v").c_str());

		// Side blur
		try
		{
			sideBlurFbo.create();
			sideBlurFbo.bind();
			sideBlurFbo.setNumOfColorAttachements(1);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
								   r.getMs().getNormalFai().getGlId(), 0);
		}
		catch(std::exception& e)
		{
			throw EXCEPTION("Cannot create side blur post-processing stage FBO: " + e.what());
		}

		sideBlur.loadRsrc("engine-rsrc/side-blur.png");
		sideBlurSProg.loadRsrc("shaders/PpsSideBlur.glsl");
	} // end blurring enabled
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

	GlStateMachineSingleton::getInstance().enable(GL_DEPTH_TEST, false);
	GlStateMachineSingleton::getInstance().enable(GL_BLEND, false);
	Renderer::setViewport(0, 0, r.getWidth(), r.getHeight());

	prePassSProg->bind();
	prePassSProg->findUniVar("isFai")->set(r.getIs().getFai(), 0);

	if(ssao.isEnabled())
	{
		prePassSProg->findUniVar("ppsSsaoFai")->set(ssao.getFai(), 1);
	}

	r.drawQuad();
}


//======================================================================================================================
// runBlur                                                                                                             =
//======================================================================================================================
void Pps::runBlur()
{
	GlStateMachineSingleton::getInstance().enable(GL_BLEND, false);

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
// runSideBlur                                                                                                         =
//======================================================================================================================
void Pps::runSideBlur()
{
	sideBlurFbo.bind();

	GlStateMachineSingleton::getInstance().enable(GL_BLEND, true);
	glBlendFunc(GL_ONE, GL_ONE);

	sideBlurSProg->bind();
	sideBlurSProg->findUniVar("tex")->set(*sideBlur, 0);

	r.drawQuad();
}


//======================================================================================================================
// runPostPass                                                                                                         =
//======================================================================================================================
void Pps::runPostPass()
{
	//
	// The actual pass
	//
	if(hdr.isEnabled())
	{
		hdr.run();
	}

	postPassFbo.bind();

	GlStateMachineSingleton::getInstance().enable(GL_DEPTH_TEST, false);
	GlStateMachineSingleton::getInstance().enable(GL_BLEND, false);
	Renderer::setViewport(0, 0, r.getWidth(), r.getHeight());

	postPassSProg->bind();
	postPassSProg->findUniVar("ppsPrePassFai")->set(prePassFai, 0);
	if(hdr.isEnabled())
	{
		postPassSProg->findUniVar("ppsHdrFai")->set(hdr.getFai(), 1);
	}

	r.drawQuad();

	//
	// Blurring
	//
	if(blurringEnabled)
	{
		runSideBlur();
		runBlur();
	}
}
