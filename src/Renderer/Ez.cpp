#include "Ez.h"
#include "Renderer.h"
#include "Core/App.h"
#include "Scene/Scene.h"
#include "RendererInitializer.h"


//==============================================================================
// init                                                                        =
//==============================================================================
void Ez::init(const RendererInitializer& initializer)
{
	enabled = initializer.ms.ez.enabled;

	if(!enabled)
	{
		return;
	}

	//
	// init FBO
	//
	try
	{
		fbo.create();
		fbo.bind();

		fbo.setNumOfColorAttachements(0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
		                       r.getMs().getDepthFai().getGlId(), 0);

		fbo.checkIfGood();

		fbo.unbind();
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Cannot create EarlyZ FBO");
	}
}


//==============================================================================
// run                                                                         =
//==============================================================================
void Ez::run()
{
	if(!enabled)
	{
		return;
	}

	const Camera& cam = r.getCamera();

	fbo.bind();

	Renderer::setViewport(0, 0, r.getWidth(), r.getHeight());

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	GlStateMachineSingleton::getInstance().enable(GL_DEPTH_TEST, true);
	GlStateMachineSingleton::getInstance().enable(GL_BLEND, false);

	glClear(GL_DEPTH_BUFFER_BIT);

	BOOST_FOREACH(const RenderableNode* node, cam.getVisibleMsRenderableNodes())
	{
		r.getSceneDrawer().renderRenderableNode(*node, cam, SceneDrawer::RPT_DEPTH);
	}

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}
