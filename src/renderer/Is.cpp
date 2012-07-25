#include "anki/renderer/Is.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/Scene.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Light.h"

#define BLEND_ENABLE 1

namespace anki {

//==============================================================================

/// Representation of the program's block. See shader for more info
struct UniformBlockData
{
	Vec4 planes;
	Vec4 limitsOfNearPlane;
	Vec4 zNearLightRadius;
	Vec4 lightPos;
	Vec4 lightDiffuseCol;
	Vec4 lightSpecularCol;
	Mat4 texProjectionMat;
};

//==============================================================================
Is::Is(Renderer* r_)
	: RenderingPass(r_), smo(r_)
{}

//==============================================================================
Is::~Is()
{}

//==============================================================================
void Is::init(const RendererInitializer& initializer)
{
	try
	{
		// Init the passes
		//
		smo.init(initializer);

		// Load the programs
		//

		// Ambient pass
		ambientPassSProg.load("shaders/IsAp.glsl");

		// point light
		pointLightSProg.load(ShaderProgramResource::createSrcCodeToCache(
			"shaders/IsLpGeneric.glsl", "#define POINT_LIGHT 1\n").c_str());

		// spot light no shadow
		spotLightNoShadowSProg.load(
			ShaderProgramResource::createSrcCodeToCache(
			"shaders/IsLpGeneric.glsl", "#define SPOT_LIGHT 1\n").c_str());

		// spot light w/t shadow
		std::string pps = std::string("#define SPOT_LIGHT 1\n"
			"#define SHADOW 1\n");
		if(/*sm.isPcfEnabled()*/ 1) // XXX
		{
			pps += "#define PCF 1\n";
		}
		spotLightShadowSProg.load(ShaderProgramResource::createSrcCodeToCache(
			"shaders/IsLpGeneric.glsl", pps.c_str()).c_str());

		// Create FBO
		//
		Renderer::createFai(r->getWidth(), r->getHeight(), GL_RGB8,
			GL_RGB, GL_UNSIGNED_INT, fai);
		fbo.create();
		fbo.setColorAttachments({&fai});
		fbo.setOtherAttachment(GL_DEPTH_STENCIL_ATTACHMENT,
			r->getMs().getDepthFai());

		if(!fbo.isComplete())
		{
			throw ANKI_EXCEPTION("Fbo not complete");
		}
		
		// Create UBO
		//
		const ShaderProgramUniformBlock& block = 
			pointLightSProg->findUniformBlock("uniforms");

		if(block.getSize() != sizeof(UniformBlockData))
		{
			throw ANKI_EXCEPTION("Uniform block size is not the expected");
		}

		ubo.create(GL_UNIFORM_BUFFER, sizeof(UniformBlockData), nullptr,
			GL_DYNAMIC_DRAW);

		ubo.setBinding(0);
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to create IS stage") << e;
	}
}

//==============================================================================
void Is::ambientPass(const Vec3& color)
{
	// set the shader
	ambientPassSProg->bind();

	// set the uniforms
	ambientPassSProg->findUniformVariable("ambientCol").set(color);
	ambientPassSProg->findUniformVariable("msFai0").set(
		r->getMs().getFai0());

	// Draw quad
	r->drawQuad();
}

//==============================================================================
void Is::pointLightPass(PointLight& light)
{
	const Camera& cam = r->getScene().getActiveCamera();

	// SMO
	smo.run(light);
	GlStateSingleton::get().disable(GL_DEPTH_TEST);

	// shader prog
	const ShaderProgram& shader = *pointLightSProg; // ensure the const-ness
	shader.bind();

	shader.findUniformVariable("msFai0").set(r->getMs().getFai0());
	shader.findUniformVariable("msDepthFai").set(
		r->getMs().getDepthFai());

	UniformBlockData data;
	data.planes = Vec4(r->getPlanes(), 0.0, 0.0);
	data.limitsOfNearPlane = Vec4(r->getLimitsOfNearPlane(),
		r->getLimitsOfNearPlane2());
	data.zNearLightRadius = Vec4(cam.getNear(), light.getRadius(), 0.0, 0.0);
	Vec3 lightPosEyeSpace = light.getWorldTransform().getOrigin().
		getTransformed(cam.getViewMatrix());
	data.lightPos = Vec4(lightPosEyeSpace, 0.0);
	data.lightDiffuseCol = light.getDiffuseColor();
	data.lightSpecularCol = light.getSpecularColor();

	ubo.write(&data, 0, sizeof(UniformBlockData));

	// render quad
	r->drawQuad();
}

//==============================================================================
void Is::run()
{
	GlStateSingleton::get().setViewport(0, 0, r->getWidth(), r->getHeight());
	fbo.bind();

	// Ambient pass
	GlStateSingleton::get().disable(GL_BLEND);
	GlStateSingleton::get().disable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	ambientPass(r->getScene().getAmbientColor());

	// render lights
#if BLEND_ENABLE
	GlStateSingleton::get().enable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
#else
	GlStateSingleton::get().disable(GL_BLEND);
#endif

	GlStateSingleton::get().enable(GL_STENCIL_TEST);

	VisibilityInfo& vi = r->getScene().getVisibilityInfo();
	for(auto it = vi.getLightsBegin();
		it != vi.getLightsEnd(); ++it)
	{
		Light& light = *(*it);
		switch(light.getLightType())
		{
		case Light::LT_SPOT:
			break;
		case Light::LT_POINT:
			pointLightPass(static_cast<PointLight&>(light));
			break;
		default:
			ANKI_ASSERT(0);
			break;
		}
	}

	GlStateSingleton::get().disable(GL_STENCIL_TEST);
	glDepthMask(GL_TRUE);
}

} // end namespace anki
