#include "anki/renderer/Ez.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/App.h"
#include "anki/scene/Scene.h"

namespace anki {

//==============================================================================
void Ez::init(const RendererInitializer& initializer)
{
	enabled = initializer.ms.ez.enabled;

	if(!enabled)
	{
		return;
	}

	maxObjectsToDraw = initializer.ms.ez.maxObjectsToDraw;

	// init FBO
	try
	{
		fbo.create();
		fbo.setOtherAttachment(GL_DEPTH_ATTACHMENT, r->getMs().getDepthFai());
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Cannot create EarlyZ FBO");
	}
}

//==============================================================================
void Ez::run()
{
	ANKI_ASSERT(enabled);

	Scene& scene = r->getScene();
	Camera& cam = scene.getActiveCamera();

	VisibilityInfo& vi = cam.getFrustumable()->getVisibilityInfo();

	U count = 0;
	for(auto it = vi.getRenderablesBegin();
		it != vi.getRenderablesEnd() && count < maxObjectsToDraw; ++it)
	{
		r->getSceneDrawer().render(cam, 0, *((*it)->getRenderable()));
		++count;
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
