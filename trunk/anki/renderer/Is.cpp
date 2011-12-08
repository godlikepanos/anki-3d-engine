#include "anki/renderer/Is.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Light.h"
#include "anki/scene/PointLight.h"
#include "anki/scene/SpotLight.h"
#include "anki/resource/LightRsrc.h"
#include "anki/core/App.h"
#include "anki/resource/LightRsrc.h"
#include "anki/renderer/Sm.h"
#include "anki/renderer/Smo.h"
#include "anki/scene/Scene.h"

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/array.hpp>


#define BLEND_ENABLE true


namespace anki {


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
		throw ANKI_EXCEPTION("Cannot create deferred shading illumination "
			"stage FBO") << e;
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
		throw ANKI_EXCEPTION("Cannot create deferred shading "
			"illumination stage additional FBO") << e;
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
	ambientPassSProg.load("shaders/IsAp.glsl");

	// point light
	pointLightSProg.load(ShaderProgram::createSrcCodeToCache(
		"shaders/IsLpGeneric.glsl", "#define POINT_LIGHT_ENABLED\n").c_str());

	// spot light no shadow
	spotLightNoShadowSProg.load(ShaderProgram::createSrcCodeToCache(
		"shaders/IsLpGeneric.glsl", "#define SPOT_LIGHT_ENABLED\n").c_str());

	// spot light w/t shadow
	std::string pps = std::string("#define SPOT_LIGHT_ENABLED\n"
	                              "#define SHADOW_ENABLED\n");
	if(sm.isPcfEnabled())
	{
		pps += "#define PCF_ENABLED\n";
	}
	spotLightShadowSProg.load(ShaderProgram::createSrcCodeToCache(
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
	ambientPassSProg->findUniformVariableByName("ambientCol").set(color);
	ambientPassSProg->findUniformVariableByName("sceneColMap").set(
		r.getMs().getDiffuseFai(), 0);

	// Draw quad
	r.drawQuad();
}


//==============================================================================
// pointLightPass                                                              =
//==============================================================================
void Is::pointLightPass(PointLight& light)
{
	const Camera& cam = r.getCamera();

	// stencil optimization
	smo.run(light);
	GlStateMachineSingleton::get().enable(GL_DEPTH_TEST, false);

	// shader prog
	const ShaderProgram& shader = *pointLightSProg; // ensure the const-ness
	shader.bind();

	shader.findUniformVariableByName("msNormalFai").set(
		r.getMs().getNormalFai(), 0);
	shader.findUniformVariableByName("msDiffuseFai").set(
		r.getMs().getDiffuseFai(), 1);
	shader.findUniformVariableByName("msSpecularFai").set(
		r.getMs().getSpecularFai(), 2);
	shader.findUniformVariableByName("msDepthFai").set(
		r.getMs().getDepthFai(), 3);
	shader.findUniformVariableByName("planes").set(r.getPlanes());
	shader.findUniformVariableByName("limitsOfNearPlane").set(
		r.getLimitsOfNearPlane());
	shader.findUniformVariableByName("limitsOfNearPlane2").set(
		r.getLimitsOfNearPlane2());
	float zNear = cam.getZNear();
	shader.findUniformVariableByName("zNear").set(zNear);
	const Vec3& origin = light.getWorldTransform().getOrigin();
	Vec3 lightPosEyeSpace = origin.getTransformed(cam.getViewMatrix());
	shader.findUniformVariableByName("lightPos").set(lightPosEyeSpace);
	shader.findUniformVariableByName("lightRadius").set(light.getRadius());
	shader.findUniformVariableByName("lightDiffuseCol").set(
		light.getDiffuseColor());
	shader.findUniformVariableByName("lightSpecularCol").set(
		light.getSpecularColor());

	// render quad
	r.drawQuad();
}


//==============================================================================
// spotLightPass                                                               =
//==============================================================================
void Is::spotLightPass(SpotLight& light)
{
	const Camera& cam = r.getCamera();
	bool withShadow = light.getCastShadow() && sm.getEnabled() &&
		(light.getVisibleMsRenderableNodes().size() > 0);

	// shadow mapping
	if(withShadow)
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

	if(withShadow)
	{
		shdr = spotLightShadowSProg.get();
	}
	else
	{
		shdr = spotLightNoShadowSProg.get();
	}

	shdr->bind();

	// bind the FAIs
	const Ms& ms = r.getMs();
	shdr->findUniformVariableByName("msNormalFai").set(ms.getNormalFai(), 0);
	shdr->findUniformVariableByName("msDiffuseFai").set(ms.getDiffuseFai(), 1);
	shdr->findUniformVariableByName("msSpecularFai").set(ms.getSpecularFai(), 2);
	shdr->findUniformVariableByName("msDepthFai").set(ms.getDepthFai(), 3);

	// the ???
	shdr->findUniformVariableByName("planes").set(r.getPlanes());
	shdr->findUniformVariableByName("limitsOfNearPlane").set(
		r.getLimitsOfNearPlane());
	shdr->findUniformVariableByName("limitsOfNearPlane2").set(
		r.getLimitsOfNearPlane2());
	float zNear = cam.getZNear();
	shdr->findUniformVariableByName("zNear").set(zNear);

	// the light params
	const Vec3& origin = light.getWorldTransform().getOrigin();
	Vec3 lightPosEyeSpace = origin.getTransformed(cam.getViewMatrix());
	shdr->findUniformVariableByName("lightPos").set(lightPosEyeSpace);
	float tmp = light.getDistance();
	shdr->findUniformVariableByName("lightRadius").set(tmp);
	shdr->findUniformVariableByName("lightDiffuseCol").set(
		light.getDiffuseColor());
	shdr->findUniformVariableByName("lightSpecularCol").set(
		light.getSpecularColor());
	shdr->findUniformVariableByName("lightTex").set(light.getTexture(), 4);

	// set texture matrix for texture & shadowmap projection
	// Bias * P_light * V_light * inv(V_cam)
	static Mat4 biasMat4(0.5, 0.0, 0.0, 0.5, 0.0, 0.5, 0.0, 0.5, 0.0, 0.0, 0.5,
		0.5, 0.0, 0.0, 0.0, 1.0);
	Mat4 texProjectionMat;
	texProjectionMat = biasMat4 * light.getCamera().getProjectionMatrix() *
		Mat4::combineTransformations(light.getCamera().getViewMatrix(),
		Mat4(cam.getWorldTransform()));
	shdr->findUniformVariableByName("texProjectionMat").set(texProjectionMat);

	// the shadowmap
	if(light.getCastShadow() && sm.getEnabled())
	{
		shdr->findUniformVariableByName("shadowMap").set(sm.getShadowMap(), 5);
		float smSize = sm.getShadowMap().getWidth();
		shdr->findUniformVariableByName("shadowMapSize").set(smSize);
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
	BOOST_FOREACH(PointLight* light,
		r.getCamera().getVisiblePointLights())
	{
		/*if(light->getVisibleMsRenderableNodes().size() == 0)
		{
			continue;
		}*/
		pointLightPass(*light);
	}

	BOOST_FOREACH(SpotLight* light, r.getCamera().getVisibleSpotLights())
	{
		/*if(light->getVisibleMsRenderableNodes() == 0)
		{
			continue;
		}*/
		spotLightPass(*light);
	}
	

	GlStateMachineSingleton::get().disable(GL_STENCIL_TEST);

	// FBO
	//fbo.unbind();

	ANKI_CHECK_GL_ERROR();
}


} // end namespace
