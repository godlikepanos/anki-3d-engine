#include "anki/renderer/Ssao.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/Camera.h"
#include "anki/scene/SceneGraph.h"

namespace anki {

//==============================================================================
struct ShaderCommonUniforms
{
	Vec4 nearPlanes;
	Vec4 limitsOfNearPlane;
};

//==============================================================================
void Ssao::createFbo(Fbo& fbo, Texture& fai, F32 width, F32 height)
{
	fai.create2dFai(width, height, GL_RED, GL_RED, GL_UNSIGNED_BYTE);

	// Set to bilinear because the blurring techniques take advantage of that
	fai.setFiltering(Texture::TFT_LINEAR);

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

	blurringIterationsCount = initializer.pps.ssao.blurringIterationsNum;

	//
	// Init the widths/heights
	//
	const F32 bQuality = initializer.pps.ssao.blurringRenderingQuality;
	const F32 mpQuality = initializer.pps.ssao.mainPassRenderingQuality;

	if(mpQuality > bQuality)
	{
		throw ANKI_EXCEPTION("SSAO blur quality shouldn't be less than "
			"main pass SSAO quality");
	}

	bWidth = bQuality * (F32)r->getWidth();
	bHeight = bQuality * (F32)r->getHeight();
	mpWidth =  mpQuality * (F32)r->getWidth();
	mpHeight =  mpQuality * (F32)r->getHeight();

	//
	// create FBOs
	//
	createFbo(hblurFbo, hblurFai, bWidth, bHeight);
	createFbo(vblurFbo, vblurFai, bWidth, bHeight);

	if(blit())
	{
		createFbo(mpFbo, mpFai, mpWidth, mpHeight);
	}

	//
	// noise map
	//
	noiseMap.load("engine_data/noise.ankitex");
	noiseMap->setFiltering(Texture::TFT_NEAREST);
	if(noiseMap->getWidth() != noiseMap->getHeight())
	{
		throw ANKI_EXCEPTION("Incorrect noisemap size");
	}

	//
	// Shaders
	//
	commonUbo.create(sizeof(ShaderCommonUniforms), nullptr);

	std::stringstream pps;

	// main pass prog
	pps << "#define NOISE_MAP_SIZE " << noiseMap->getWidth()
		<< "\n#define WIDTH " << mpWidth
		<< "\n#define HEIGHT " << mpHeight
		<< "\n#define USE_MRT " << ANKI_RENDERER_USE_MRT
		<< "\n";
	ssaoSProg.load(ShaderProgramResource::createSrcCodeToCache(
		"shaders/PpsSsao.glsl", pps.str().c_str()).c_str());

	ssaoSProg->findUniformBlock("commonBlock").setBinding(0);

	// blurring progs
	const char* SHADER_FILENAME = "shaders/VariableSamplingBlurGeneric.glsl";

	pps.clear();
	pps << "#define HPASS\n"
		"#define COL_R\n"
		"#define IMG_DIMENSION " << bHeight << "\n"
		"#define SAMPLES 7\n";
	hblurSProg.load(ShaderProgramResource::createSrcCodeToCache(
		SHADER_FILENAME, pps.str().c_str()).c_str());

	pps.clear();
	pps << "#define VPASS\n"
		"#define COL_R\n"
		"#define IMG_DIMENSION " << bWidth << "\n"
		"#define SAMPLES 7\n";
	vblurSProg.load(ShaderProgramResource::createSrcCodeToCache(
		SHADER_FILENAME, pps.str().c_str()).c_str());
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
	ANKI_ASSERT(enabled);

	const Camera& cam = r->getSceneGraph().getActiveCamera();

	GlStateSingleton::get().disable(GL_BLEND);
	GlStateSingleton::get().disable(GL_DEPTH_TEST);

	// 1st pass
	//
	if(blit())
	{
		mpFbo.bind();
		GlStateSingleton::get().setViewport(0, 0, mpWidth, mpHeight);
	}
	else
	{
		vblurFbo.bind();
		GlStateSingleton::get().setViewport(0, 0, bWidth, bHeight);
	}
	r->clearAfterBindingFbo(GL_COLOR_BUFFER_BIT);
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
		commonUboUpdateTimestamp = getGlobTimestamp();
	}

	// msDepthFai
	ssaoSProg->findUniformVariable("msDepthFai").set(
		r->getMs().getDepthFai());

	// noiseMap
	ssaoSProg->findUniformVariable("noiseMap").set(*noiseMap);

	// msGFai
#if ANKI_RENDERER_USE_MRT
	ssaoSProg->findUniformVariable("msGFai").set(r->getMs().getFai1());
#else
	ssaoSProg->findUniformVariable("msGFai").set(r->getMs().getFai0());
#endif

	// Draw
	r->drawQuad();

	// Blit from main pass FBO to vertical pass FBO
	if(blit())
	{
		vblurFbo.blit(mpFbo,
			0, 0, mpWidth, mpHeight,
			0, 0, bWidth, bHeight,
			GL_COLOR_BUFFER_BIT, GL_LINEAR);

		// Set the correct viewport
		GlStateSingleton::get().setViewport(0, 0, bWidth, bHeight);
	}

	// Blurring passes
	//
	for(U32 i = 0; i < blurringIterationsCount; i++)
	{
		// hpass
		hblurFbo.bind();
		r->clearAfterBindingFbo(GL_COLOR_BUFFER_BIT);
		hblurSProg->bind();
		if(i == 0)
		{
			hblurSProg->findUniformVariable("img").set(vblurFai);
		}
		r->drawQuad();

		// vpass
		vblurFbo.bind();
		r->clearAfterBindingFbo(GL_COLOR_BUFFER_BIT);
		vblurSProg->bind();
		if(i == 0)
		{
			vblurSProg->findUniformVariable("img").set(hblurFai);
		}
		r->drawQuad();
	}
}

} // end namespace anki
