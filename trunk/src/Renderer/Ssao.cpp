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
		fai.createEmpty2D(width, height, GL_RED, GL_RED, GL_FLOAT);
		fai.setRepeat(false);
		fai.setTexParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		fai.setTexParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);

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
	ssaoSProg.loadRsrc("shaders/PpsSsao-2.glsl");

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
	
	//
	// Geom
	//
	
	float quadVertCoords[][2] = {{1.0, 1.0}, {0.0, 1.0}, {0.0, 0.0}, {1.0, 0.0}};
	quadPositionsVbo.create(GL_ARRAY_BUFFER, sizeof(quadVertCoords), quadVertCoords, GL_STATIC_DRAW);

	ushort quadVertIndeces[2][3] = {{0, 1, 3}, {1, 2, 3}};
	quadVertIndecesVbo.create(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadVertIndeces), quadVertIndeces, GL_STATIC_DRAW);

	viewVectorsVbo.create(GL_ARRAY_BUFFER, 4 * sizeof(Vec3), NULL, GL_DYNAMIC_DRAW);

	vao.create();
	vao.attachArrayBufferVbo(quadPositionsVbo, 0, 2, GL_FLOAT, false, 0, NULL);
	vao.attachArrayBufferVbo(viewVectorsVbo, 1, 3, GL_FLOAT, false, 0, NULL);
	vao.attachElementArrayBufferVbo(quadVertIndecesVbo);
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
	
	boost::array<Vec3, 4> viewVectors;
	boost::array<float, 2> gScreenSize = {{width, height}};
	Is::calcViewVectors(gScreenSize, cam.getInvProjectionMatrix(), viewVectors);
	viewVectorsVbo.write(&viewVectors[0]);
	
	/*BOOST_FOREACH(Vec3 v, viewVectors)
	{
		std::cout << v << std::endl;
	}
	std::cout << "----------------" << std::endl;*/
	
	//std::cout << planes << std::endl;

	ssaoFbo.bind();
	ssaoSProg->bind();
	
	Vec2 planes;
	Is::calcPlanes(Vec2(r.getCamera().getZNear(), r.getCamera().getZFar()), planes);
	if(ssaoSProg->uniVarExists("planes"))
		ssaoSProg->findUniVar("planes")->set(&planes);

	Vec2 limitsOfNearPlane;
	const PerspectiveCamera& pcam = static_cast<const PerspectiveCamera&>(cam);
	limitsOfNearPlane.y() = cam.getZNear() * tan(0.5 * pcam.getFovY());
	limitsOfNearPlane.x() = limitsOfNearPlane.y() * (pcam.getFovX() / pcam.getFovY());
	if(ssaoSProg->uniVarExists("limitsOfNearPlane"))
		ssaoSProg->findUniVar("limitsOfNearPlane")->set(&limitsOfNearPlane);

	if(ssaoSProg->uniVarExists("msDepthFai"))
		ssaoSProg->findUniVar("msDepthFai")->set(r.getMs().getDepthFai(), 0);

	if(ssaoSProg->uniVarExists("noiseMap"))
		ssaoSProg->findUniVar("noiseMap")->set(*noiseMap, 1);

	if(ssaoSProg->uniVarExists("msNormalFai"))
		ssaoSProg->findUniVar("msNormalFai")->set(r.getMs().getNormalFai(), 2);

	Vec2 screenSize(width, height);
	if(ssaoSProg->uniVarExists("screenSize"))
		ssaoSProg->findUniVar("screenSize")->set(&screenSize);

	float noiseMapSize = noiseMap->getWidth();
	if(ssaoSProg->uniVarExists("noiseMapSize"))
		ssaoSProg->findUniVar("noiseMapSize")->set(&noiseMapSize);

	if(ssaoSProg->uniVarExists("msPosFai"))
		ssaoSProg->findUniVar("msPosFai")->set(r.getMs().posFai, 3);

	vao.bind();
	glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_SHORT, 0);
	vao.unbind();


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

