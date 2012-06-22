#include "anki/renderer/Ez.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/App.h"
#include "anki/scene/Scene.h"
#include "anki/renderer/RendererInitializer.h"

namespace anki {

//==============================================================================
void Ez::init(const RendererInitializer& initializer)
{
	enabled = initializer.ms.ez.enabled;

	if(!enabled)
	{
		return;
	}

	// init FBO
	try
	{
		fbo.create();
		fbo.setOtherAttachment(GL_DEPTH_ATTACHMENT, 
			GL_TEXTURE_2D, r->getMs().getDepthFai().getGlId());
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Cannot create EarlyZ FBO");
	}
}

//==============================================================================
void Ez::run()
{
	if(!enabled)
	{
		return;
	}

	/*Camera& cam = r.getCamera();

	fbo.bind();

	GlStateMachineSingleton::get().setViewport(0, 0,
		r.getWidth(), r.getHeight());

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	GlStateMachineSingleton::get().enable(GL_DEPTH_TEST, true);
	GlStateMachineSingleton::get().enable(GL_BLEND, false);

	glClear(GL_DEPTH_BUFFER_BIT);

	for(RenderableNode* node, cam.getVisibleMsRenderableNodes())
	{
		r.getSceneDrawer().renderRenderableNode(cam, 1, *node);
	}

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);*/
}

} // end namespace anki
