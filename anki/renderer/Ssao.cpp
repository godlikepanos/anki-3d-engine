#include <boost/lexical_cast.hpp>
#include "anki/renderer/Ssao.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/Camera.h"
#include "anki/renderer/RendererInitializer.h"
#include "anki/scene/PerspectiveCamera.h"


namespace anki {


//==============================================================================
// createFbo                                                                   =
//==============================================================================
void Ssao::createFbo(Fbo& fbo, Texture& fai)
{
	try
	{
		int width = renderingQuality * r.getWidth();
		int height = renderingQuality * r.getHeight();

		// create
		fbo.create();
		fbo.bind();

		// inform in what buffers we draw
		fbo.setNumOfColorAttachements(1);

		// create the texes
		Renderer::createFai(width, height, GL_RED, GL_RED, GL_FLOAT, fai);

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
		throw ANKI_EXCEPTION_R("Cannot create deferred shading post-processing "
			"stage SSAO blur FBO", e);
	}
}


//==============================================================================
// init                                                                        =
//==============================================================================
void Ssao::init(const RendererInitializer& initializer)
{
	enabled = initializer.pps.ssao.enabled;

	if(!enabled)
	{
		return;
	}

	renderingQuality = initializer.pps.ssao.renderingQuality;
	blurringIterationsNum = initializer.pps.ssao.blurringIterationsNum;

	// create FBOs
	createFbo(ssaoFbo, ssaoFai);
	createFbo(hblurFbo, hblurFai);
	createFbo(vblurFbo, fai);

	//
	// Shaders
	//

	// first pass prog
	ssaoSProg.load("shaders/PpsSsao.glsl");

	// blurring progs
	const char* SHADER_FILENAME = "shaders/GaussianBlurGeneric.glsl";

	std::string pps = "#define HPASS\n#define COL_R\n";
	hblurSProg.load(
		ShaderProgram::createSrcCodeToCache(SHADER_FILENAME, 
		pps.c_str()).c_str());

	pps = "#define VPASS\n#define COL_R\n";
	vblurSProg.load(
		ShaderProgram::createSrcCodeToCache(SHADER_FILENAME, 
		pps.c_str()).c_str());

	//
	// noise map
	//

	noiseMap.load("engine-rsrc/noise.png");
	noiseMap->setRepeat(true);
}


//==============================================================================
// run                                                                         =
//==============================================================================
void Ssao::run()
{
	if(!enabled)
	{
		return;
	}

	int width = renderingQuality * r.getWidth();
	int height = renderingQuality * r.getHeight();
	const Camera& cam = r.getCamera();

	GlStateMachineSingleton::get().enable(GL_BLEND, false);
	GlStateMachineSingleton::get().enable(GL_DEPTH_TEST, false);


	GlStateMachineSingleton::get().setViewport(0, 0, width, height);

	//
	// 1st pass
	//
	
	ssaoFbo.bind();
	ssaoSProg->bind();
	
	// planes
	ssaoSProg->getUniformVariableByName("planes").set(&r.getPlanes());

	// limitsOfNearPlane
	ssaoSProg->getUniformVariableByName("limitsOfNearPlane").set(
		&r.getLimitsOfNearPlane());

	// limitsOfNearPlane2
	ssaoSProg->getUniformVariableByName("limitsOfNearPlane2").set(
		&r.getLimitsOfNearPlane2());

	// zNear
	float zNear = cam.getZNear();
	ssaoSProg->getUniformVariableByName("zNear").set(&zNear);

	// msDepthFai
	ssaoSProg->getUniformVariableByName("msDepthFai").set(
		r.getMs().getDepthFai(), 0);

	// noiseMap
	ssaoSProg->getUniformVariableByName("noiseMap").set(*noiseMap, 1);

	// noiseMapSize
	float noiseMapSize = noiseMap->getWidth();
	ssaoSProg->getUniformVariableByName("noiseMapSize").set(&noiseMapSize);

	// screenSize
	Vec2 screenSize(width * 2, height * 2);
	ssaoSProg->getUniformVariableByName("screenSize").set(&screenSize);

	// msNormalFai
	ssaoSProg->getUniformVariableByName("msNormalFai").set(
		r.getMs().getNormalFai(), 2);

	r.drawQuad();


	//
	// Blurring passes
	//
	hblurFai.setRepeat(false);
	fai.setRepeat(false);
	for(uint i = 0; i < blurringIterationsNum; i++)
	{
		// hpass
		hblurFbo.bind();
		hblurSProg->bind();
		if(i == 0)
		{
			hblurSProg->getUniformVariableByName("img").set(ssaoFai, 0);
		}
		else
		{
			hblurSProg->getUniformVariableByName("img").set(fai, 0);
		}
		float tmp = width;
		hblurSProg->getUniformVariableByName("imgDimension").set(&tmp);
		r.drawQuad();

		// vpass
		vblurFbo.bind();
		vblurSProg->bind();
		vblurSProg->getUniformVariableByName("img").set(hblurFai, 0);
		tmp = height;
		vblurSProg->getUniformVariableByName("imgDimension").set(&tmp);
		r.drawQuad();
	}

	// end
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind the window framebuffer
}


} // end namespace
