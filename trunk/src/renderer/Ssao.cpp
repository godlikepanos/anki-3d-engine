#include <boost/lexical_cast.hpp>
#include "anki/renderer/Ssao.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Scene.h"

namespace anki {

//==============================================================================
void Ssao::createFbo(Fbo& fbo, Texture& fai)
{
	try
	{
		Renderer::createFai(width, height, GL_RED, GL_RED, GL_FLOAT, fai);

		fbo.create();
		fbo.setColorAttachments({&fai});
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Cannot create deferred shading post-processing "
			"stage SSAO blur FBO") << e;
	}
}

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

	width = renderingQuality * r->getWidth();
	height = renderingQuality * r->getHeight();

	// create FBOs
	createFbo(ssaoFbo, ssaoFai);
	createFbo(hblurFbo, hblurFai);
	createFbo(vblurFbo, fai);

	// Shaders
	//

	// first pass prog
	ssaoSProg.load("shaders/PpsSsao.glsl");

	// blurring progs
	const char* SHADER_FILENAME = "shaders/GaussianBlurGeneric.glsl";

	std::string pps = "#define HPASS\n#define COL_R\n#define IMG_DIMENSION "
		+ std::to_string(width) + ".0\n";
	hblurSProg.load(SHADER_FILENAME, pps.c_str());

	pps = "#define VPASS\n#define COL_R\n#define IMG_DIMENSION "
		+ std::to_string(width) + ".0 \n";
	vblurSProg.load(SHADER_FILENAME, pps.c_str());

	// noise map
	//
	noiseMap.load("engine-rsrc/noise.png");
}

//==============================================================================
void Ssao::run()
{
	if(!enabled)
	{
		return;
	}

	const Camera& cam = r->getScene().getActiveCamera();

	GlStateSingleton::get().disable(GL_BLEND);
	GlStateSingleton::get().disable(GL_DEPTH_TEST);
	GlStateSingleton::get().setViewport(0, 0, width, height);

	// 1st pass
	//
	
	ssaoFbo.bind();
	ssaoSProg.bind();
	
	// planes
	ssaoSProg.findUniformVariable("planes").set(r->getPlanes());

	// limitsOfNearPlane
	ssaoSProg.findUniformVariable("limitsOfNearPlane").set(
		r->getLimitsOfNearPlane());

	// limitsOfNearPlane2
	ssaoSProg.findUniformVariable("limitsOfNearPlane2").set(
		r->getLimitsOfNearPlane2());

	// zNear
	ssaoSProg.findUniformVariable("zNear").set(cam.getNear());

	// msDepthFai
	ssaoSProg.findUniformVariable("msDepthFai").set(
		r->getMs().getDepthFai());

	// noiseMap
	ssaoSProg.findUniformVariable("noiseMap").set(*noiseMap);

	// noiseMapSize
	ssaoSProg.findUniformVariable("noiseMapSize").set(
		noiseMap->getWidth());

	// screenSize
	Vec2 screenSize(width * 2, height * 2);
	ssaoSProg.findUniformVariable("screenSize").set(screenSize);

	// msNormalFai
	ssaoSProg.findUniformVariable("msNormalFai").set(
		r->getMs().getFai0());

	r->drawQuad();

	vblurSProg.bind();
	vblurSProg.findUniformVariable("img").set(hblurFai);

	// Blurring passes
	//
	for(uint32_t i = 0; i < blurringIterationsNum; i++)
	{
		// hpass
		hblurFbo.bind();
		hblurSProg.bind();
		if(i == 0)
		{
			hblurSProg.findUniformVariable("img").set(ssaoFai);
		}
		else if(i == 1)
		{
			hblurSProg.findUniformVariable("img").set(fai);
		}
		r->drawQuad();

		// vpass
		vblurFbo.bind();
		vblurSProg.bind();
		r->drawQuad();
	}
}

} // end namespace
