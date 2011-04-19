#include <boost/lexical_cast.hpp>
#include "Ssao.h"
#include "Renderer.h"
#include "Camera.h"
#include "RendererInitializer.h"
#include "PerspectiveCamera.h"


//======================================================================================================================
// createFbo                                                                                                           =
//======================================================================================================================
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
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fai.getGlId(), 0);

		// test if success
		fbo.checkIfGood();

		// unbind
		fbo.unbind();
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Cannot create deferred shading post-processing stage SSAO blur FBO");
	}
}


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Ssao::init(const RendererInitializer& initializer)
{
	enabled = initializer.pps.ssao.enabled;

	if(!enabled)
		return;

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
	ssaoSProg.loadRsrc("shaders/PpsSsao.glsl");

	// blurring progs
	const char* SHADER_FILENAME = "shaders/GaussianBlurGeneric.glsl";

	std::string pps = "#define HPASS\n#define COL_R\n";
	std::string prefix = "HorizontalR";
	hblurSProg.loadRsrc(ShaderProg::createSrcCodeToCache(SHADER_FILENAME, pps.c_str(), prefix.c_str()).c_str());

	pps = "#define VPASS\n#define COL_R\n";
	prefix = "VerticalR";
	vblurSProg.loadRsrc(ShaderProg::createSrcCodeToCache(SHADER_FILENAME, pps.c_str(), prefix.c_str()).c_str());

	//
	// noise map
	//

	/// @todo fix this crap
	// load noise map and disable temporally the texture compression and enable mipmapping
	bool texCompr = Texture::compressionEnabled;
	bool mipmaping = Texture::mipmappingEnabled;
	Texture::compressionEnabled = false;
	Texture::mipmappingEnabled = true;
	noiseMap.loadRsrc("engine-rsrc/noise.png");
	noiseMap->setTexParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	noiseMap->setTexParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
	//noise_map->setTexParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//noise_map->setTexParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	Texture::compressionEnabled = texCompr;
	Texture::mipmappingEnabled = mipmaping;
}


//======================================================================================================================
// run                                                                                                                 =
//======================================================================================================================
void Ssao::run()
{
	int width = renderingQuality * r.getWidth();
	int height = renderingQuality * r.getHeight();
	const Camera& cam = r.getCamera();

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);


	Renderer::setViewport(0, 0, width, height);

	//
	// 1st pass
	//
	
	ssaoFbo.bind();
	ssaoSProg->bind();
	
	// planes
	Vec2 planes;
	Is::calcPlanes(Vec2(r.getCamera().getZNear(), r.getCamera().getZFar()), planes);
	ssaoSProg->findUniVar("planes")->set(&planes);

	// limitsOfNearPlane
	Vec2 limitsOfNearPlane;
	ASSERT(cam.getType() == Camera::CT_PERSPECTIVE);
	const PerspectiveCamera& pcam = static_cast<const PerspectiveCamera&>(cam);
	limitsOfNearPlane.y() = cam.getZNear() * tan(0.5 * pcam.getFovY());
	limitsOfNearPlane.x() = limitsOfNearPlane.y() * (pcam.getFovX() / pcam.getFovY());
	ssaoSProg->findUniVar("limitsOfNearPlane")->set(&limitsOfNearPlane);

	// limitsOfNearPlane2
	limitsOfNearPlane *= 2;
	ssaoSProg->findUniVar("limitsOfNearPlane2")->set(&limitsOfNearPlane);

	// zNear
	float zNear = cam.getZNear();
	ssaoSProg->findUniVar("zNear")->set(&zNear);

	// msDepthFai
	ssaoSProg->findUniVar("msDepthFai")->set(r.getMs().getDepthFai(), 0);

	// noiseMap
	ssaoSProg->findUniVar("noiseMap")->set(*noiseMap, 1);

	// noiseMapSize
	float noiseMapSize = noiseMap->getWidth();
	ssaoSProg->findUniVar("noiseMapSize")->set(&noiseMapSize);

	// screenSize
	Vec2 screenSize(width, height);
	ssaoSProg->findUniVar("screenSize")->set(&screenSize);

	// msNormalFai
	ssaoSProg->findUniVar("msNormalFai")->set(r.getMs().getNormalFai(), 2);

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
			hblurSProg->findUniVar("img")->set(ssaoFai, 0);
		}
		else
		{
			hblurSProg->findUniVar("img")->set(fai, 0);
		}
		float tmp = width;
		hblurSProg->findUniVar("imgDimension")->set(&tmp);
		r.drawQuad();

		// vpass
		vblurFbo.bind();
		vblurSProg->bind();
		vblurSProg->findUniVar("img")->set(hblurFai, 0);
		tmp = height;
		vblurSProg->findUniVar("imgDimension")->set(&tmp);
		r.drawQuad();
	}

	// end
	Fbo::unbind();
}

