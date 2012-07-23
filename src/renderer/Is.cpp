#include "anki/renderer/Is.h"
#include "anki/renderer/Renderer.h"

#define BLEND_ENABLE true

namespace anki {

/// Representation of the program's block
struct UniformBlockData
{
	vec2 planes;
	vec2 limitsOfNearPlane;
	vec2 limitsOfNearPlane2;
	float zNear; float padding0;
	float lightRadius; float padding1;
	float shadowMapSize; float padding2;
	vec3 lightPos; float padding3;
	vec3 lightDiffuseCol; float padding4;
	vec3 lightSpecularCol; float padding5;
	mat4 texProjectionMat;
};

//==============================================================================
Is::Is(Renderer* r_)
	: RenderingPass(r_)
{}

//==============================================================================
Is::~Is()
{}

//==============================================================================
void Is::init(const RendererInitializer& /*initializer*/)
{
	try
	{
		// Load the passes
		//
		// XXX

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
		fbo.create();
		fbo.setColorAttachments({fai});

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

		ubo.create(GL_UNIFORM_BUFFER, blockSize, nullptr, GL_STATIC_DRAW);

		ubo.setBinding(0);
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to create IS stage") << e;
	}
}

//==============================================================================
void Is::pointLightPass(PointLight& light)
{
	const Camera& cam = r.getCamera();

	// XXX SMO
	GlStateSingleton::get().enable(GL_DEPTH_TEST, false);

	// shader prog
	const ShaderProgram& shader = *pointLightSProg; // ensure the const-ness
	shader.bind();

	shader.findUniformVariableByName("msFai0")->set(r->getMs().getFai0());
	shader.findUniformVariableByName("msDepthFai")->set(
		r->getMs().getDepthFai());

	UniformBlockData data;
	data.planes = r->getPlanes();
	data.limitsOfNearPlane = r->.getLimitsOfNearPlane();
	data.limitsOfNearPlane2 = r->getLimitsOfNearPlane();
	data.zNear = cam.getZNear();
	Vec3 lightPosEyeSpace = origin.getTransformed(cam.getViewMatrix());
	data.lightPos = lightPosEyeSpace;
	data.lightRadius = light.getRadius();
	data.lightDiffuseCol = light.getDiffuseColor();
	data.lightSpecularCol = light.getSpecularColor();

	ubo.write(&data, 0, sizeof(UniformBlockData));

	// render quad
	r->drawQuad();
}

//==============================================================================
void Is::run()
{
	/// TODO
}

} // end namespace anki
