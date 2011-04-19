#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/array.hpp>

#include "Is.h"
#include "Renderer.h"
#include "Camera.h"
#include "Light.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "LightRsrc.h"
#include "App.h"
#include "LightRsrc.h"
#include "Sm.h"
#include "Smo.h"
#include "Scene.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Is::Is(Renderer& r_):
	RenderingPass(r_),
	sm(r_),
	smo(r_)
{}


//======================================================================================================================
// calcViewVectors                                                                                                     =
//======================================================================================================================
void Is::calcViewVectors(const boost::array<float, 2>& screenSize, const Mat4& invProjectionMat,
                         boost::array<Vec3, 4>& viewVectors)
{
	uint w = screenSize[0];
	uint h = screenSize[1];

	// From right up and CC wise to right down, Just like we render the quad
	uint pixels[4][2] = {{w, h}, {0, h}, {0, 0}, {w, 0}};
	boost::array<uint, 4> viewport = {{0, 0, w, h}};

	for(int i = 0; i < 4; i++)
	{
		/*
		Original Code:
		Renderer::unProject(pixels[i][0], pixels[i][1], 10, cam.getViewMatrix(), cam.getProjectionMatrix(), viewport,
		                    viewVectors[i].x, viewVectors[i].y, viewVectors[i].z);
		viewVectors[i] = cam.getViewMatrix() * viewVectors[i];
		The original code is the above 3 lines. The optimized follows:
		*/

		Vec3 vec;
		vec.x() = (2.0 * (pixels[i][0] - viewport[0])) / viewport[2] - 1.0;
		vec.y() = (2.0 * (pixels[i][1] - viewport[1])) / viewport[3] - 1.0;
		vec.z() = 1.0;

		viewVectors[i] = vec.getTransformed(invProjectionMat);
		// end of optimized code
	}
}


//======================================================================================================================
// calcViewVectors                                                                                                     =
//======================================================================================================================
void Is::calcViewVectors()
{
	boost::array<Vec3, 4> viewVectors;
	boost::array<float, 2> screenSize = {{r.getWidth(), r.getHeight()}};

	calcViewVectors(screenSize, r.getCamera().getInvProjectionMatrix(), viewVectors);

	viewVectorsVbo.write(&viewVectors[0]);
}


//======================================================================================================================
// calcPlanes                                                                                                          =
//======================================================================================================================
void Is::calcPlanes(const Vec2& cameraRange, Vec2& planes)
{
	float zNear = cameraRange.x();
	float zFar = cameraRange.y();

	planes.x() = zFar / (zNear - zFar);
	planes.y() = (zFar * zNear) / (zNear -zFar);
}


//======================================================================================================================
// initFbo                                                                                                             =
//======================================================================================================================
void Is::initFbo()
{
	try
	{
		// create FBO
		fbo.create();
		fbo.bind();

		// init the stencil render buffer
		glGenRenderbuffers(1, &stencilRb);
		glBindRenderbuffer(GL_RENDERBUFFER, stencilRb);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX, r.getWidth(), r.getHeight());
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencilRb);

		// inform in what buffers we draw
		fbo.setNumOfColorAttachements(1);

		// create the FAI
		Renderer::createFai(r.getWidth(), r.getHeight(), GL_RGB, GL_RGB, GL_FLOAT, fai);

		// attach
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fai.getGlId(), 0);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, r.ms.depthFai.getGlId(), 0);

		// test if success
		fbo.checkIfGood();

		// unbind
		fbo.unbind();
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Cannot create deferred shading illumination stage FBO: " + e.what());
	}
}


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Is::init(const RendererInitializer& initializer)
{
	// init passes
	smo.init(initializer);
	sm.init(initializer);

	// load the shaders
	ambientPassSProg.loadRsrc("shaders/IsAp.glsl");

	// point light
	pointLightSProg.loadRsrc(ShaderProg::createSrcCodeToCache("shaders/IsLpGeneric.glsl",
	                                                          "#define POINT_LIGHT_ENABLED\n",
	                                                          "Point").c_str());

	// spot light no shadow
	spotLightNoShadowSProg.loadRsrc(ShaderProg::createSrcCodeToCache("shaders/IsLpGeneric.glsl",
	                                                                 "#define SPOT_LIGHT_ENABLED\n",
	                                                                 "Spot_NoShadow").c_str());

	// spot light w/t shadow
	std::string pps = std::string("#define SPOT_LIGHT_ENABLED\n"
	                              "#define SHADOW_ENABLED\n");
	std::string prefix = "Spot_Shadow";
	if(sm.isPcfEnabled())
	{
		pps += "#define PCF_ENABLED\n";
		prefix += "_Pcf";
	}
	spotLightShadowSProg.loadRsrc(ShaderProg::createSrcCodeToCache("shaders/IsLpGeneric.glsl", pps.c_str(),
	                                                               prefix.c_str()).c_str());

	// init the rest
	initFbo();

	// The VBOs and VAOs
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
// drawLightPassQuad                                                                                                   =
//======================================================================================================================
void Is::drawLightPassQuad() const
{
	vao.bind();
	glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_SHORT, 0);
	vao.unbind();
	ON_GL_FAIL_THROW_EXCEPTION();
}


//======================================================================================================================
// ambientPass                                                                                                         =
//======================================================================================================================
void Is::ambientPass(const Vec3& color)
{
	glDisable(GL_BLEND);

	// set the shader
	ambientPassSProg->bind();

	// set the uniforms
	ambientPassSProg->findUniVar("ambientCol")->set(&color);
	ambientPassSProg->findUniVar("sceneColMap")->set(r.getMs().getDiffuseFai(), 0);

	// Draw quad
	r.drawQuad();
}


//======================================================================================================================
// pointLightPass                                                                                                      =
//======================================================================================================================
void Is::pointLightPass(const PointLight& light)
{
	const Camera& cam = r.getCamera();

	// stencil optimization
	smo.run(light);

	// shader prog
	const ShaderProg& shader = *pointLightSProg; // ensure the const-ness
	shader.bind();

	shader.findUniVar("msNormalFai")->set(r.getMs().getNormalFai(), 0);
	shader.findUniVar("msDiffuseFai")->set(r.getMs().getDiffuseFai(), 1);
	shader.findUniVar("msSpecularFai")->set(r.getMs().getSpecularFai(), 2);
	shader.findUniVar("msDepthFai")->set(r.getMs().getDepthFai(), 3);
	shader.findUniVar("planes")->set(&planes);
	Vec3 lightPosEyeSpace = light.getWorldTransform().getOrigin().getTransformed(cam.getViewMatrix());
	shader.findUniVar("lightPos")->set(&lightPosEyeSpace);
	shader.findUniVar("lightRadius")->set(&light.getRadius());
	shader.findUniVar("lightDiffuseCol")->set(&light.getDiffuseCol());
	shader.findUniVar("lightSpecularCol")->set(&light.getSpecularCol());

	// render quad
	drawLightPassQuad();
}


//======================================================================================================================
// spotLightPass                                                                                                       =
//======================================================================================================================
void Is::spotLightPass(const SpotLight& light)
{
	const Camera& cam = r.getCamera();

	// shadow mapping
	if(light.castsShadow() && sm.isEnabled())
	{
		float dist = (light.getWorldTransform().getOrigin() - cam.getWorldTransform().getOrigin()).getLength();
		sm.run(light.getCamera(), dist);

		// restore the IS FBO
		fbo.bind();

		// and restore blending and depth test
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glDisable(GL_DEPTH_TEST);
		Renderer::setViewport(0, 0, r.getWidth(), r.getHeight());
	}

	// stencil optimization
	smo.run(light);

	// set the texture
	//light.getTexture().setRepeat(false);

	// shader prog
	const ShaderProg* shdr;

	if(light.castsShadow() && sm.isEnabled())
	{
		shdr = spotLightShadowSProg.get();
	}
	else
	{
		shdr = spotLightNoShadowSProg.get();
	}

	shdr->bind();

	// bind the FAIs
	shdr->findUniVar("msNormalFai")->set(r.getMs().getNormalFai(), 0);
	shdr->findUniVar("msDiffuseFai")->set(r.getMs().getDiffuseFai(), 1);
	shdr->findUniVar("msSpecularFai")->set(r.getMs().getSpecularFai(), 2);
	shdr->findUniVar("msDepthFai")->set(r.getMs().getDepthFai(), 3);

	// the planes
	shdr->findUniVar("planes")->set(&planes);

	// the light params
	Vec3 lightPosEyeSpace = light.getWorldTransform().getOrigin().getTransformed(cam.getViewMatrix());
	shdr->findUniVar("lightPos")->set(&lightPosEyeSpace);
	float tmp = light.getDistance();
	shdr->findUniVar("lightRadius")->set(&tmp);
	shdr->findUniVar("lightDiffuseCol")->set(&light.getDiffuseCol());
	shdr->findUniVar("lightSpecularCol")->set(&light.getSpecularCol());
	shdr->findUniVar("lightTex")->set(light.getTexture(), 4);

	// set texture matrix for texture & shadowmap projection
	// Bias * P_light * V_light * inv(V_cam)
	static Mat4 biasMat4(0.5, 0.0, 0.0, 0.5, 0.0, 0.5, 0.0, 0.5, 0.0, 0.0, 0.5, 0.5, 0.0, 0.0, 0.0, 1.0);
	Mat4 texProjectionMat;
	texProjectionMat = biasMat4 * light.getCamera().getProjectionMatrix() *
	                   Mat4::combineTransformations(light.getCamera().getViewMatrix(), Mat4(cam.getWorldTransform()));
	shdr->findUniVar("texProjectionMat")->set(&texProjectionMat);

	// the shadowmap
	if(light.castsShadow() && sm.isEnabled())
	{
		shdr->findUniVar("shadowMap")->set(sm.getShadowMap(), 5);
		float smSize = sm.getShadowMap().getWidth();
		shdr->findUniVar("shadowMapSize")->set(&smSize);
	}

	// render quad
	drawLightPassQuad();
}


//======================================================================================================================
// run                                                                                                                 =
//======================================================================================================================
void Is::run()
{
	// FBO
	fbo.bind();

	// OGL stuff
	Renderer::setViewport(0, 0, r.getWidth(), r.getHeight());

	glDisable(GL_DEPTH_TEST);

	// ambient pass
	ambientPass(SceneSingleton::getInstance().getAmbientCol());

	// light passes
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_STENCIL_TEST);

	calcViewVectors();
	calcPlanes(Vec2(r.getCamera().getZNear(), r.getCamera().getZFar()), planes);

	// for all lights
	BOOST_FOREACH(const PointLight* light, r.getCamera().getVisiblePointLights())
	{
		pointLightPass(*light);
	}
	
	BOOST_FOREACH(const SpotLight* light, r.getCamera().getVisibleSpotLights())
	{
		spotLightPass(*light);
	}
	

	glDisable(GL_STENCIL_TEST);

	// FBO
	fbo.unbind();

	ON_GL_FAIL_THROW_EXCEPTION();
}
