#include <boost/lexical_cast.hpp>
#include "anki/renderer/Ssao.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Scene.h"

namespace anki {

//==============================================================================
struct ShaderCommonUniforms
{
	Vec4 nearPlanes;
	Vec4 limitsOfNearPlane;
};

//==============================================================================
void Ssao::createFbo(Fbo& fbo, Texture& fai)
{

	Renderer::createFai(width, height, GL_RED, GL_RED, GL_FLOAT, fai);

	fbo.create();
	fbo.setColorAttachments({&fai});

	if(!fbo.isComplete())
	{
		throw ANKI_EXCEPTION("Fbo not complete");
	}
}

//==============================================================================
void Ssao::initInternal(const RendererInitializer& initializer)
{
	enabled = initializer.pps.ssao.enabled;

	if(!enabled)
	{
		return;
	}

	F32 renderingQuality = initializer.pps.ssao.renderingQuality 
		* initializer.renderingQuality;
	blurringIterationsCount = initializer.pps.ssao.blurringIterationsNum;

	width = renderingQuality * (F32)initializer.width;
	height = renderingQuality * (F32)initializer.height;

	//
	// create FBOs
	//
	createFbo(hblurFbo, hblurFai);
	createFbo(vblurFbo, vblurFai);

	//
	// noise map
	//
	noiseMap.load("engine-rsrc/noise.png");
	noiseMap->setFiltering(Texture::TFT_NEAREST);
	if(noiseMap->getWidth() != noiseMap->getHeight())
	{
		throw ANKI_EXCEPTION("Incorrect noisemap size");
	}

	//
	// Shaders
	//
	commonUbo.create(sizeof(ShaderCommonUniforms), nullptr);

	std::string pps;

	// first pass prog
	pps = "#define NOISE_MAP_SIZE " + std::to_string(noiseMap->getWidth())
		+ "\n#define WIDTH " + std::to_string(width)
		+ "\n#define HEIGHT " + std::to_string(height) + "\n";
	ssaoSProg.load(ShaderProgramResource::createSrcCodeToCache(
		"shaders/PpsSsao.glsl", pps.c_str()).c_str());

	// blurring progs
	const char* SHADER_FILENAME = "shaders/GaussianBlurGeneric.glsl";

	pps = "#define HPASS\n"
		"#define COL_R\n"
		"#define IMG_DIMENSION " + std::to_string(width) + ".0\n";
	hblurSProg.load(ShaderProgramResource::createSrcCodeToCache(
		SHADER_FILENAME, pps.c_str()).c_str());

	pps = "#define VPASS\n"
		"#define COL_R\n"
		"#define IMG_DIMENSION " + std::to_string(width) + ".0 \n";
	vblurSProg.load(ShaderProgramResource::createSrcCodeToCache(
		SHADER_FILENAME, pps.c_str()).c_str());
}

//==============================================================================
void Ssao::init(const RendererInitializer& initializer)
{
	try
	{
		initInternal(initializer);
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to init PPS SSAO") << e;
	}
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
	
	vblurFbo.bind();
	ssaoSProg->bind();
	commonUbo.setBinding(0);

	// Write common block
	if(commonUboUpdateTimestamp < r->getPlanesUpdateTimestamp()
		|| commonUboUpdateTimestamp == 1)
	{
		ShaderCommonUniforms blk;
		blk.nearPlanes = Vec4(cam.getNear(), 0.0, r->getPlanes().x(),
			r->getPlanes().y());
		blk.limitsOfNearPlane = Vec4(r->getLimitsOfNearPlane(),
			r->getLimitsOfNearPlane2());

		commonUbo.write(&blk);
		commonUboUpdateTimestamp = Timestamp::getTimestamp();
	}

	// msDepthFai
	ssaoSProg->findUniformVariable("msDepthFai").set(
		r->getMs().getDepthFai());

	// noiseMap
	ssaoSProg->findUniformVariable("noiseMap").set(*noiseMap);

	// msGFai
	ssaoSProg->findUniformVariable("msGFai").set(r->getMs().getFai0());

	r->drawQuad();

	// Blurring passes
	//
	for(U32 i = 0; i < blurringIterationsCount; i++)
	{
		// hpass
		hblurFbo.bind();
		hblurSProg->bind();
		if(i == 0)
		{
			hblurSProg->findUniformVariable("img").set(vblurFai);
		}
		r->drawQuad();

		// vpass
		vblurFbo.bind();
		vblurSProg->bind();
		if(i == 0)
		{
			vblurSProg->findUniformVariable("img").set(hblurFai);
		}
		r->drawQuad();
	}
}

} // end namespace anki
