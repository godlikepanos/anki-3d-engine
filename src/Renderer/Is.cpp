/**
 * @file
 *
 * Illumination stage
 */

#include <boost/lexical_cast.hpp>
#include "Renderer.h"
#include "Camera.h"
#include "Light.h"
#include "LightProps.h"
#include "App.h"
#include "Scene.h"
#include "LightProps.h"


//======================================================================================================================
// CalcViewVector                                                                                                      =
//======================================================================================================================
void Renderer::Is::calcViewVector()
{
	const Camera& cam = *r.cam;

	const uint& w = r.width;
	const uint& h = r.height;

	// From right up and CC wise to right down, Just like we render the quad
	uint pixels[4][2]={{w,h}, {0,h}, {0, 0}, {w, 0}};

	uint viewport[4]={ 0, 0, w, h };

	for(int i=0; i<4; i++)
	{
		/*
		 * Original Code:
		 * Renderer::unProject(pixels[i][0], pixels[i][1], 10, cam.getViewMatrix(), cam.getProjectionMatrix(), viewport,
		 *                     viewVectors[i].x, viewVectors[i].y, viewVectors[i].z);
		 * viewVectors[i] = cam.getViewMatrix() * viewVectors[i];
		 * The original code is the above 3 lines. The optimized follows:
		 */

		Vec3 vec;
		vec.x = (2.0*(pixels[i][0]-viewport[0]))/viewport[2] - 1.0;
		vec.y = (2.0*(pixels[i][1]-viewport[1]))/viewport[3] - 1.0;
		vec.z = 1.0;

		viewVectors[i] = vec.getTransformed(cam.getInvProjectionMatrix());
		// end of optimized code
	}
}


//======================================================================================================================
// calcPlanes                                                                                                          =
//======================================================================================================================
void Renderer::Is::calcPlanes()
{
	const Camera& cam = *r.cam;

	planes.x = cam.getZFar() / (cam.getZNear() - cam.getZFar());
	planes.y = (cam.getZFar() * cam.getZNear()) / (cam.getZNear() -cam.getZFar());
}


//======================================================================================================================
// initFbo                                                                                                             =
//======================================================================================================================
void Renderer::Is::initFbo()
{
	// create FBO
	fbo.create();
	fbo.bind();

	// init the stencil render buffer
	glGenRenderbuffers(1, &stencilRb);
	glBindRenderbuffer(GL_RENDERBUFFER, stencilRb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX, r.width, r.height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencilRb);

	// inform in what buffers we draw
	fbo.setNumOfColorAttachements(1);

	// create the txtrs
	if(!fai.createEmpty2D(r.width, r.height, GL_RGB, GL_RGB, GL_FLOAT, false))
		FATAL("Cannot create deferred shading illumination stage FAI");

	// attach
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fai.getGlId(), 0);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, r.ms.depthFai.getGlId(), 0);

	// test if success
	if(!fbo.isGood())
		FATAL("Cannot create deferred shading illumination stage FBO");

	// unbind
	fbo.unbind();
}


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Renderer::Is::init()
{
	// load the shaders
	ambientPassSProg.loadRsrc("shaders/IsAp.glsl");
	ambientColUniVar = ambientPassSProg->findUniVar("ambientCol");
	sceneColMapUniVar = ambientPassSProg->findUniVar("sceneColMap");

	// point light
	pointLightSProg.loadRsrc(ShaderProg::createSrcCodeToCache("shaders/IsLpGeneric.glsl", "#define POINT_LIGHT_ENABLED\n",
	                                                          "Point").c_str());
	pointLightSProgUniVars.msNormalFai = pointLightSProg->findUniVar("msNormalFai");
	pointLightSProgUniVars.msDiffuseFai = pointLightSProg->findUniVar("msDiffuseFai");
	pointLightSProgUniVars.msSpecularFai = pointLightSProg->findUniVar("msSpecularFai");
	pointLightSProgUniVars.msDepthFai = pointLightSProg->findUniVar("msDepthFai");
	pointLightSProgUniVars.planes = pointLightSProg->findUniVar("planes");
	pointLightSProgUniVars.lightPos = pointLightSProg->findUniVar("lightPos");
	pointLightSProgUniVars.lightInvRadius = pointLightSProg->findUniVar("lightInvRadius");
	pointLightSProgUniVars.lightDiffuseCol = pointLightSProg->findUniVar("lightDiffuseCol");
	pointLightSProgUniVars.lightSpecularCol = pointLightSProg->findUniVar("lightSpecularCol");


	// spot light no shadow
	spotLightNoShadowSProg.loadRsrc(ShaderProg::createSrcCodeToCache("shaders/IsLpGeneric.glsl",
	                                                                 "#define SPOT_LIGHT_ENABLED\n",
	                                                                 "SpotNoShadow").c_str());
	spotLightNoShadowSProgUniVars.msNormalFai = spotLightNoShadowSProg->findUniVar("msNormalFai");
	spotLightNoShadowSProgUniVars.msDiffuseFai = spotLightNoShadowSProg->findUniVar("msDiffuseFai");
	spotLightNoShadowSProgUniVars.msSpecularFai = spotLightNoShadowSProg->findUniVar("msSpecularFai");
	spotLightNoShadowSProgUniVars.msDepthFai = spotLightNoShadowSProg->findUniVar("msDepthFai");
	spotLightNoShadowSProgUniVars.planes = spotLightNoShadowSProg->findUniVar("planes");
	spotLightNoShadowSProgUniVars.lightPos = spotLightNoShadowSProg->findUniVar("lightPos");
	spotLightNoShadowSProgUniVars.lightInvRadius = spotLightNoShadowSProg->findUniVar("lightInvRadius");
	spotLightNoShadowSProgUniVars.lightDiffuseCol = spotLightNoShadowSProg->findUniVar("lightDiffuseCol");
	spotLightNoShadowSProgUniVars.lightSpecularCol = spotLightNoShadowSProg->findUniVar("lightSpecularCol");
	spotLightNoShadowSProgUniVars.lightTex = spotLightNoShadowSProg->findUniVar("lightTex");
	spotLightNoShadowSProgUniVars.texProjectionMat = spotLightNoShadowSProg->findUniVar("texProjectionMat");


	// spot light w/t shadow
	string pps = string("\n#define SPOT_LIGHT_ENABLED\n#define SHADOW_ENABLED\n") +
	             "#define SHADOWMAP_SIZE " + lexical_cast<string>(sm.resolution) + "\n";
	string prefix = "SpotShadowSms" + lexical_cast<string>(sm.resolution);
	if(sm.pcfEnabled)
	{
		pps += "#define PCF_ENABLED\n";
		prefix += "Pcf";
	}
	spotLightShadowSProg.loadRsrc(ShaderProg::createSrcCodeToCache("shaders/IsLpGeneric.glsl", pps.c_str(),
	                                                               prefix.c_str()).c_str());
	spotLightShadowSProgUniVars.msNormalFai = spotLightShadowSProg->findUniVar("msNormalFai");
	spotLightShadowSProgUniVars.msDiffuseFai = spotLightShadowSProg->findUniVar("msDiffuseFai");
	spotLightShadowSProgUniVars.msSpecularFai = spotLightShadowSProg->findUniVar("msSpecularFai");
	spotLightShadowSProgUniVars.msDepthFai = spotLightShadowSProg->findUniVar("msDepthFai");
	spotLightShadowSProgUniVars.planes = spotLightShadowSProg->findUniVar("planes");
	spotLightShadowSProgUniVars.lightPos = spotLightShadowSProg->findUniVar("lightPos");
	spotLightShadowSProgUniVars.lightInvRadius = spotLightShadowSProg->findUniVar("lightInvRadius");
	spotLightShadowSProgUniVars.lightDiffuseCol = spotLightShadowSProg->findUniVar("lightDiffuseCol");
	spotLightShadowSProgUniVars.lightSpecularCol = spotLightShadowSProg->findUniVar("lightSpecularCol");
	spotLightShadowSProgUniVars.lightTex = spotLightShadowSProg->findUniVar("lightTex");
	spotLightShadowSProgUniVars.texProjectionMat = spotLightShadowSProg->findUniVar("texProjectionMat");
	spotLightShadowSProgUniVars.shadowMap = spotLightShadowSProg->findUniVar("shadowMap");


	// init the rest
	initFbo();
	smo.init();

	if(sm.enabled)
		sm.init();
}


//======================================================================================================================
// ambientPass                                                                                                         =
//======================================================================================================================
void Renderer::Is::ambientPass(const Vec3& color)
{
	glDisable(GL_BLEND);

	// set the shader
	ambientPassSProg->bind();

	// set the uniforms
	ambientColUniVar->setVec3(&color);
	sceneColMapUniVar->setTexture(r.ms.diffuseFai, 0);

	// Draw quad
	Renderer::drawQuad(0);
}


//======================================================================================================================
// pointLightPass                                                                                                      =
//======================================================================================================================
void Renderer::Is::pointLightPass(const PointLight& light)
{
	const Camera& cam = *r.cam;

	// frustum test
	bsphere_t sphere(light.getWorldTransform().getOrigin(), light.getRadius());
	if(!cam.insideFrustum(sphere)) return;

	// stencil optimization
	smo.run(light);

	// shader prog
	const ShaderProg& shader = *pointLightSProg; // ensure the const-ness
	shader.bind();

	pointLightSProgUniVars.msNormalFai->setTexture(r.ms.normalFai, 0);
	pointLightSProgUniVars.msDiffuseFai->setTexture(r.ms.diffuseFai, 1);
	pointLightSProgUniVars.msSpecularFai->setTexture(r.ms.specularFai, 2);
	pointLightSProgUniVars.msDepthFai->setTexture(r.ms.depthFai, 3);
	pointLightSProgUniVars.planes->setVec2(&planes);
	Vec3 lightPosEyeSpace = light.getWorldTransform().getOrigin().getTransformed(cam.getViewMatrix());
	pointLightSProgUniVars.lightPos->setVec3(&lightPosEyeSpace);
	pointLightSProgUniVars.lightInvRadius->setFloat(1.0/light.getRadius());
	Vec3 ll = light.lightProps->getDiffuseColor();
	pointLightSProgUniVars.lightDiffuseCol->setVec3(&light.lightProps->getDiffuseColor());
	pointLightSProgUniVars.lightSpecularCol->setVec3(&light.lightProps->getSpecularColor());


	// render quad
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, &Renderer::quadVertCoords[0]);
	glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, &viewVectors[0]);

	glDrawArrays(GL_QUADS, 0, 4);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}


//======================================================================================================================
// spotLightPass                                                                                                       =
//======================================================================================================================
void Renderer::Is::spotLightPass(const SpotLight& light)
{
	const Camera& cam = *r.cam;

	// frustum test
	if(!cam.insideFrustum(light.camera)) return;

	// shadow mapping
	if(light.castsShadow && sm.enabled)
	{
		sm.run(light.camera);

		// restore the IS FBO
		fbo.bind();

		// and restore blending and depth test
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glDisable(GL_DEPTH_TEST);
		Renderer::setViewport(0, 0, r.width, r.height);
	}

	// stencil optimization
	smo.run(light);

	// set the texture
	if(light.lightProps->getTexture() == NULL)
	{
		ERROR("No texture is attached to the light. lightProps name: " << light.lightProps->getRsrcName());
		return;
	}

	light.lightProps->getTexture()->setRepeat(false);

	// shader prog
	const ShaderProg* shdr; // because of the huge name
	const UniVars* uniVars;

	if(light.castsShadow && sm.enabled)
	{
		shdr = spotLightShadowSProg.get();
		uniVars = &spotLightShadowSProgUniVars;
	}
	else
	{
		shdr = spotLightNoShadowSProg.get();
		uniVars = &spotLightNoShadowSProgUniVars;
	}

	shdr->bind();

	// bind the FAIs
	uniVars->msNormalFai->setTexture(r.ms.normalFai, 0);
	uniVars->msDiffuseFai->setTexture(r.ms.diffuseFai, 1);
	uniVars->msSpecularFai->setTexture(r.ms.specularFai, 2);
	uniVars->msDepthFai->setTexture(r.ms.depthFai, 3);

	// the planes
	uniVars->planes->setVec2(&planes);

	// the light params
	Vec3 lightPosEyeSpace = light.getWorldTransform().getOrigin().getTransformed(cam.getViewMatrix());
	uniVars->lightPos->setVec3(&lightPosEyeSpace);
	uniVars->lightInvRadius->setFloat(1.0/light.getDistance());
	uniVars->lightDiffuseCol->setVec3(&light.lightProps->getDiffuseColor());
	uniVars->lightSpecularCol->setVec3(&light.lightProps->getSpecularColor());
	uniVars->lightTex->setTexture(*light.lightProps->getTexture(), 4);

	// set texture matrix for texture & shadowmap projection
	// Bias * P_light * V_light * inv(V_cam)
	static Mat4 biasMat4(0.5, 0.0, 0.0, 0.5, 0.0, 0.5, 0.0, 0.5, 0.0, 0.0, 0.5, 0.5, 0.0, 0.0, 0.0, 1.0);
	Mat4 texProjectionMat;
	texProjectionMat = biasMat4 * light.camera.getProjectionMatrix() *
	                   Mat4::combineTransformations(light.camera.getViewMatrix(), Mat4(cam.getWorldTransform()));
	uniVars->texProjectionMat->setMat4(&texProjectionMat);

	// the shadowmap
	if(light.castsShadow && sm.enabled)
	{
		uniVars->shadowMap->setTexture(sm.shadowMap, 5);
	}

	// render quad
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, &Renderer::quadVertCoords[0]);
	glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, &viewVectors[0]);

	glDrawArrays(GL_QUADS, 0, 4);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}


//======================================================================================================================
// run                                                                                                                 =
//======================================================================================================================
void Renderer::Is::run()
{
	// FBO
	fbo.bind();

	// OGL stuff
	Renderer::setViewport(0, 0, r.width, r.height);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);

	// ambient pass
	ambientPass(app->getScene()->getAmbientCol());

	// light passes
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_STENCIL_TEST);

	calcViewVector();
	calcPlanes();

	// for all lights
	for(uint i=0; i<app->getScene()->lights.size(); i++)
	{
		const Light& light = *app->getScene()->lights[i];
		switch(light.type)
		{
			case Light::LT_POINT:
			{
				const PointLight& pointl = static_cast<const PointLight&>(light);
				pointLightPass(pointl);
				break;
			}

			case Light::LT_SPOT:
			{
				const SpotLight& projl = static_cast<const SpotLight&>(light);
				spotLightPass(projl);
				break;
			}

			default:
				DEBUG_ERR(1);
		}
	}

	glDisable(GL_STENCIL_TEST);

	// FBO
	fbo.unbind();
}
