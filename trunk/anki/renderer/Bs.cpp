#include <boost/ptr_container/ptr_vector.hpp>
#include "anki/renderer/Bs.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/Scene.h"
#include "anki/resource/ShaderProgram.h"
#include "anki/resource/Model.h"
#include "anki/scene/ModelNode.h"
#include "anki/resource/Material.h"
#include "anki/resource/Mesh.h"


//==============================================================================
// Destructor                                                                  =
//==============================================================================
Bs::~Bs()
{}


//==============================================================================
// createFbo                                                                   =
//==============================================================================
void Bs::createFbo()
{
	try
	{
		fbo.create();
		fbo.bind();

		fbo.setNumOfColorAttachements(1);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, r.getPps().getPrePassFai().getGlId(), 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
			GL_TEXTURE_2D, r.getMs().getDepthFai().getGlId(), 0);

		fbo.checkIfGood();

		fbo.unbind();
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to create blending stage FBO");
	}
}


//==============================================================================
// createRefractFbo                                                            =
//==============================================================================
void Bs::createRefractFbo()
{
	try
	{
		refractFbo.create();
		refractFbo.bind();

		refractFbo.setNumOfColorAttachements(1);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, refractFai.getGlId(), 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
			GL_TEXTURE_2D, r.getMs().getDepthFai().getGlId(), 0);

		refractFbo.checkIfGood();

		refractFbo.unbind();
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to create blending stage refract FBO");
	}
}


//==============================================================================
// init                                                                        =
//==============================================================================
void Bs::init(const RendererInitializer& /*initializer*/)
{
	createFbo();
	Renderer::createFai(r.getWidth(), r.getHeight(), GL_RGBA8, GL_RGBA,
		GL_FLOAT, refractFai);
	createRefractFbo();
	refractSProg.loadRsrc("shaders/BsRefract.glsl");
}


//==============================================================================
// run                                                                         =
//==============================================================================
void Bs::run()
{
	/// @todo
}
