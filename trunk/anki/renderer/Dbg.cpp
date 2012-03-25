#include "anki/renderer/Dbg.h"
#include "anki/renderer/Renderer.h"
#include "anki/renderer/RendererInitializer.h"


namespace anki {


//==============================================================================
Dbg::Dbg(Renderer& r_)
	: SwitchableRenderingPass(r_)
{}


//==============================================================================
void Dbg::init(const RendererInitializer& initializer)
{
	enabled = initializer.dbg.enabled;

	//
	// FBO
	//
	try
	{
		fbo.create();
		fbo.bind();

		fbo.setNumOfColorAttachements(1);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, r.getPps().getPostPassFai().getGlId(), 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			GL_TEXTURE_2D, r.getMs().getDepthFai().getGlId(), 0);

		fbo.checkIfGood();
		fbo.unbind();
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Cannot create debug FBO") << e;
	}
}


//==============================================================================
void Dbg::run()
{
	if(!enabled)
	{
		return;
	}

	/// TODO
}


} // end namespace
