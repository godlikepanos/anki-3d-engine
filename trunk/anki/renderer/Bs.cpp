#include "anki/renderer/Bs.h"
#include "anki/renderer/Renderer.h"
#include "anki/resource/ShaderProgramResource.h"


namespace anki {


//==============================================================================
Bs::~Bs()
{}


//==============================================================================
void Bs::createFbo()
{
	try
	{
		fbo.create();
		fbo.bind();

		fbo.setColorAttachments({&r->getPps().getPrePassFai()});
		fbo.setOtherAttachment(GL_DEPTH_STENCIL_ATTACHMENT, 
			r->getMs().getDepthFai());

		fbo.checkIfGood();
		fbo.unbind();
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to create blending stage FBO") << e;
	}
}


//==============================================================================
void Bs::createRefractFbo()
{
	try
	{
		refractFbo.create();
		refractFbo.bind();

		refractFbo.setColorAttachments({&refractFai});
		refractFbo.setOtherAttachment(GL_DEPTH_STENCIL_ATTACHMENT, 
			r->getMs().getDepthFai());

		refractFbo.checkIfGood();
		refractFbo.unbind();
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to create blending stage refract FBO") 
			<< e;
	}
}


//==============================================================================
void Bs::init(const RendererInitializer& /*initializer*/)
{
	createFbo();
	Renderer::createFai(r->getWidth(), r->getHeight(), GL_RGBA8, GL_RGBA,
		GL_FLOAT, refractFai);
	createRefractFbo();
	refractSProg.load("shaders/BsRefract.glsl");
}


//==============================================================================
void Bs::run()
{
	/// @todo
}


} // end namespace
