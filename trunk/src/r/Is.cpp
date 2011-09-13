#include "Is.h"
#include "Renderer.h"
#include "scene/Camera.h"
#include "scene/Light.h"
#include "scene/PointLight.h"
#include "scene/SpotLight.h"
#include "rsrc/LightRsrc.h"
#include "core/App.h"
#include "rsrc/LightRsrc.h"
#include "Sm.h"
#include "Smo.h"
#include "scene/Scene.h"

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/array.hpp>


#define BLEND_ENABLE true


//==============================================================================
// Constructor                                                                 =
//==============================================================================
Is::Is(Renderer& r_):
	RenderingPass(r_),
	sm(r_),
	smo(r_)
{}


//==============================================================================
// initFbo                                                                     =
//==============================================================================
void Is::initFbo()
{
	try
	{
		// create FBO
		fbo.create();
		fbo.bind();

		// inform in what buffers we draw
		fbo.setNumOfColorAttachements(1);

		// create the FAI
		Renderer::createFai(r.getWidth(), r.getHeight(), GL_RGB, GL_RGB,
			GL_FLOAT, fai);

		// attach
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, fai.getGlId(), 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
			GL_TEXTURE_2D, copyMsDepthFai.getGlId(), 0);

		// test if success
		fbo.checkIfGood();

		// unbind
		fbo.unbind();
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Cannot create deferred shading illumination "
			"stage FBO: " + e.what());
	}
}


//==============================================================================
// initCopy                                                                    =
//==============================================================================
void Is::initCopy()
{
	try
	{
		// Read
		readFbo.create();
		readFbo.bind();
		readFbo.setNumOfColorAttachements(0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
			GL_TEXTURE_2D, r.getMs().getDepthFai().getGlId(), 0);
		readFbo.unbind();

		// Write
		Renderer::createFai(r.getWidth(), r.getHeight(), GL_DEPTH24_STENCIL8,
			GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, copyMsDepthFai);

		writeFbo.create();
		writeFbo.bind();
		writeFbo.setNumOfColorAttachements(0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
			GL_TEXTURE_2D, copyMsDepthFai.getGlId(), 0);
		writeFbo.unbind();
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Cannot create deferred shading illumination stage "
			"additional FBO: " + e.what());
	}
}


//==============================================================================
// init                                                                        =
//==============================================================================
void Is::init(const RendererInitializer& initializer)
{
	// init passes
	smo.init(initializer);
	sm.init(initializer);

	// load the shaders
	ambientPassSProg.loadRsrc("shaders/IsAp.glsl");

	// point light
	pointLightSProg.loadRsrc(ShaderProgram::createSrcCodeToCache(
		"shaders/IsLpGeneric.glsl", "#define POINT_LIGHT_ENABLED\n").c_str());

	// spot light no shadow
	spotLightNoShadowSProg.loadRsrc(ShaderProgram::createSrcCodeToCache(
		"shaders/IsLpGeneric.glsl", "#define SPOT_LIGHT_ENABLED\n").c_str());

	// spot light w/t shadow
	std::string pps = std::string("#define SPOT_LIGHT_ENABLED\n"
	                              "#define SHADOW_ENABLED\n");
	if(sm.isPcfEnabled())
	{
		pps += "#define PCF_ENABLED\n";
	}
	spotLightShadowSProg.loadRsrc(ShaderProgram::createSrcCodeToCache(
		"shaders/IsLpGeneric.glsl", pps.c_str()).c_str());

	// init the rest
	initCopy();
	initFbo();
}


//==============================================================================
// ambientPass                                                                 =
//==============================================================================
void Is::ambientPass(const Vec3& color)
{
	GlStateMachineSingleton::get().enable(GL_BLEND, false);

	// set the shader
	ambientPassSProg->bind();

	// set the uniforms
	ambientPassSProg->getUniformVariableByName("ambientCol").set(&color);
	ambientPassSProg->getUniformVariableByName("sceneColMap").set(
		r.getMs().getDiffuseFai(), 0);

	// Draw quad
	r.drawQuad();
}


//==============================================================================
// pointLightPass                                                              =
//==============================================================================
void Is::pointLightPass(const PointLight& light)
{
	const Camera& cam = r.getCamera();

	// stencil optimization
	smo.run(light);
	GlStateMachineSingleton::get().enable(GL_DEPTH_TEST, false);

	// shader prog
	const ShaderProgram& shader = *pointLightSProg; // ensure the const-ness
	shader.bind();

	shader.getUniformVariableByName("msNormalFai").set(
		r.getMs().getNormalFai(), 0);
	shader.getUniformVariableByName("msDiffuseFai").set(
		r.getMs().getDiffuseFai(), 1);
	shader.getUniformVariableByName("msSpecularFai").set(
		r.getMs().getSpecularFai(), 2);
	shader.getUniformVariableByName("msDepthFai").set(
		r.getMs().getDepthFai(), 3);
	shader.getUniformVariableByName("planes").set(&r.getPlanes());
	shader.getUniformVariableByName("limitsOfNearPlane").set(
		&r.getLimitsOfNearPlane());
	shader.getUniformVariableByName("limitsOfNearPlane2").set(
		&r.getLimitsOfNearPlane2());
	float zNear = cam.getZNear();
	shader.getUniformVariableByName("zNear").set(&zNear);
	const Vec3& origin = light.getWorldTransform().getOrigin();
	Vec3 lightPosEyeSpace = origin.getTransformed(cam.getViewMatrix());
	shader.getUniformVariableByName("lightPos").set(&lightPosEyeSpace);
	shader.getUniformVariableByName("lightRadius").set(light.getRadius());
	shader.getUniformVariableByName("lightDiffuseCol").set(
		&light.getDiffuseColor());
	shader.getUniformVariableByName("lightSpecularCol").set(
		&light.getSpecularColor());

	// render quad
	r.drawQuad();
}


//==============================================================================
// spotLightPass                                                               =
//==============================================================================
void Is::spotLightPass(const SpotLight& light)
{
	const Camera& cam = r.getCamera();

	// shadow mapping
	if(light.getCastShadow() && sm.isEnabled())
	{
		Vec3 zAxis = light.getWorldTransform().getRotation().getColumn(2);
		LineSegment seg(light.getWorldTransform().getOrigin(),
			-zAxis * light.getCamera().getZFar());

		const Plane& plane = cam.getWSpaceFrustumPlane(Camera::FP_NEAR);

		float dist = seg.testPlane(plane);

		sm.run(light, dist);

		// restore the IS FBO
		fbo.bind();

		// and restore blending and depth test
		GlStateMachineSingleton::get().enable(GL_BLEND, BLEND_ENABLE);
		glBlendFunc(GL_ONE, GL_ONE);
		GlStateMachineSingleton::get().enable(GL_DEPTH_TEST, false);
		GlStateMachineSingleton::get().setViewport(0, 0,
			r.getWidth(), r.getHeight());
	}

	// stencil optimization
	smo.run(light);
	GlStateMachineSingleton::get().enable(GL_DEPTH_TEST, false);

	// set the texture
	//light.getTexture().setRepeat(false);

	// shader prog
	const ShaderProgram* shdr;

	if(light.getCastShadow() && sm.isEnabled())
	{
		shdr = spotLightShadowSProg.get();
	}
	else
	{
		shdr = spotLightNoShadowSProg.get();
	}

	shdr->bind();

	// bind the FAIs
	shdr->getUniformVariableByName("msNormalFai").set(
		r.getMs().getNormalFai(), 0);
	shdr->getUniformVariableByName("msDiffuseFai").set(
		r.getMs().getDiffuseFai(), 1);
	shdr->getUniformVariableByName("msSpecularFai").set(
		r.getMs().getSpecularFai(), 2);
	shdr->getUniformVariableByName("msDepthFai").set(
		r.getMs().getDepthFai(), 3);

	// the ???
	shdr->getUniformVariableByName("planes").set(&r.getPlanes());
	shdr->getUniformVariableByName("limitsOfNearPlane").set(
		&r.getLimitsOfNearPlane());
	shdr->getUniformVariableByName("limitsOfNearPlane2").set(
		&r.getLimitsOfNearPlane2());
	float zNear = cam.getZNear();
	shdr->getUniformVariableByName("zNear").set(&zNear);

	// the light params
	const Vec3& origin = light.getWorldTransform().getOrigin();
	Vec3 lightPosEyeSpace = origin.getTransformed(cam.getViewMatrix());
	shdr->getUniformVariableByName("lightPos").set(&lightPosEyeSpace);
	float tmp = light.getDistance();
	shdr->getUniformVariableByName("lightRadius").set(&tmp);
	shdr->getUniformVariableByName("lightDiffuseCol").set(
		&light.getDiffuseColor());
	shdr->getUniformVariableByName("lightSpecularCol").set(
		&light.getSpecularColor());
	shdr->getUniformVariableByName("lightTex").set(light.getTexture(), 4);

	// set texture matrix for texture & shadowmap projection
	// Bias * P_light * V_light * inv(V_cam)
	static Mat4 biasMat4(0.5, 0.0, 0.0, 0.5, 0.0, 0.5, 0.0, 0.5, 0.0, 0.0, 0.5,
		0.5, 0.0, 0.0, 0.0, 1.0);
	Mat4 texProjectionMat;
	texProjectionMat = biasMat4 * light.getCamera().getProjectionMatrix() *
		Mat4::combineTransformations(light.getCamera().getViewMatrix(),
		Mat4(cam.getWorldTransform()));
	shdr->getUniformVariableByName("texProjectionMat").set(&texProjectionMat);

	// the shadowmap
	if(light.getCastShadow() && sm.isEnabled())
	{
		shdr->getUniformVariableByName("shadowMap").set(sm.getShadowMap(), 5);
		float smSize = sm.getShadowMap().getWidth();
		shdr->getUniformVariableByName("shadowMapSize").set(&smSize);
	}

	// render quad
	r.drawQuad();
}


//==============================================================================
// copyDepth                                                                   =
//==============================================================================
void Is::copyDepth()
{
	readFbo.bind(GL_READ_FRAMEBUFFER);
	writeFbo.bind(GL_DRAW_FRAMEBUFFER);
	glBlitFramebuffer(0, 0, r.getWidth(), r.getHeight(), 0, 0, r.getWidth(),
	    r.getHeight(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}


//==============================================================================
// run                                                                         =
//==============================================================================
void Is::run()
{
	// OGL stuff
	GlStateMachineSingleton::get().setViewport(0, 0,
		r.getWidth(), r.getHeight());

	// Copy
	if(r.getFramesNum() % 2 == 0)
	{
		copyDepth();
	}

	// FBO
	fbo.bind();

	// ambient pass
	GlStateMachineSingleton::get().enable(GL_DEPTH_TEST, false);
	ambientPass(SceneSingleton::get().getAmbientColor());

	// light passes
	GlStateMachineSingleton::get().enable(GL_BLEND, BLEND_ENABLE);
	glBlendFunc(GL_ONE, GL_ONE);
	GlStateMachineSingleton::get().enable(GL_STENCIL_TEST);

	// for all lights
	BOOST_FOREACH(const PointLight* light,
		r.getCamera().getVisiblePointLights())
	{
		pointLightPass(*light);
	}
	
	BOOST_FOREACH(const SpotLight* light, r.getCamera().getVisibleSpotLights())
	{
		spotLightPass(*light);
	}
	

	GlStateMachineSingleton::get().disable(GL_STENCIL_TEST);

	// FBO
	//fbo.unbind();

	ON_GL_FAIL_THROW_EXCEPTION();
}
