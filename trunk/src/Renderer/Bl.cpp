#include "Bl.h"
#include "RendererInitializer.h"
#include "Renderer.h"
#include "Resources/ShaderProgram.h"


namespace R {


//==============================================================================
// Constructor                                                                 =
//==============================================================================
Bl::Bl(Renderer& r_):
	RenderingPass(r_)
{}


//==============================================================================
// init                                                                        =
//==============================================================================
void Bl::init(const RendererInitializer& initializer)
{
	enabled = initializer.pps.bl.enabled;
	blurringIterationsNum = initializer.pps.bl.blurringIterationsNum;
	sideBlurFactor = initializer.pps.bl.sideBlurFactor;

	if(!enabled)
	{
		return;
	}

	// Horizontal
	try
	{
		Renderer::createFai(r.getWidth(), r.getHeight(), GL_RGB, GL_RGB,
			GL_FLOAT, blurFai);

		hBlurFbo.create();
		hBlurFbo.bind();
		hBlurFbo.setNumOfColorAttachements(1);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, blurFai.getGlId(), 0);
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Cannot create horizontal blur post-processing stage "
			"FBO: " + e.what());
	}

	hBlurSProg.loadRsrc(ShaderProgram::createSrcCodeToCache(
		"shaders/PpsBlurGeneric.glsl", "#define HPASS\n").c_str());

	// Vertical
	try
	{
		vBlurFbo.create();
		vBlurFbo.bind();
		vBlurFbo.setNumOfColorAttachements(1);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, r.getPps().getPostPassFai().getGlId(), 0);
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Cannot create vertical blur post-processing stage "
			"FBO: " + e.what());
	}

	vBlurSProg.loadRsrc(ShaderProgram::createSrcCodeToCache(
		"shaders/PpsBlurGeneric.glsl", "#define VPASS\n").c_str());

	// Side blur
	try
	{
		sideBlurFbo.create();
		sideBlurFbo.bind();
		sideBlurFbo.setNumOfColorAttachements(1);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, r.getMs().getNormalFai().getGlId(), 0);
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Cannot create side blur post-processing stage FBO: " +
			e.what());
	}

	sideBlurMap.loadRsrc("engine-rsrc/side-blur.png");
	sideBlurSProg.loadRsrc("shaders/PpsSideBlur.glsl");
}


//==============================================================================
// runSideBlur                                                                 =
//==============================================================================
void Bl::runSideBlur()
{
	if(sideBlurFactor == 0.0)
	{
		return;
	}

	sideBlurFbo.bind();

	GlStateMachineSingleton::getInstance().enable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	sideBlurSProg->bind();
	sideBlurSProg->getUniformVariableByName("tex").set(*sideBlurMap, 0);
	sideBlurSProg->getUniformVariableByName("factor").set(&sideBlurFactor);

	r.drawQuad();
}


//==============================================================================
// runBlur                                                                     =
//==============================================================================
void Bl::runBlur()
{
	GlStateMachineSingleton::getInstance().disable(GL_BLEND);

	for(uint i = 0; i < blurringIterationsNum; i++)
	{
		// hpass
		hBlurFbo.bind();

		hBlurSProg->bind();
		hBlurSProg->getUniformVariableByName("img").set(
			r.getPps().getPostPassFai(), 0);
		hBlurSProg->getUniformVariableByName("msNormalFai").set(
			r.getMs().getNormalFai(), 1);
		float tmp = r.getWidth();
		hBlurSProg->getUniformVariableByName("imgDimension").set(&tmp);

		r.drawQuad();

		// vpass
		vBlurFbo.bind();

		vBlurSProg->bind();
		vBlurSProg->getUniformVariableByName("img").set(blurFai, 0);
		vBlurSProg->getUniformVariableByName("msNormalFai").set(
			r.getMs().getNormalFai(), 1);
		tmp = r.getHeight();
		vBlurSProg->getUniformVariableByName("imgDimension").set(&tmp);

		r.drawQuad();
	}
}


//==============================================================================
// run                                                                         =
//==============================================================================
void Bl::run()
{
	if(!enabled)
	{
		return;
	}

	runSideBlur();
	runBlur();
}


} // end namespace
