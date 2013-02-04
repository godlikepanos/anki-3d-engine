#include "anki/renderer/Bs.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/Scene.h"
#include "anki/scene/Camera.h"

namespace anki {

//==============================================================================
Bs::~Bs()
{}

//==============================================================================
void Bs::init(const RendererInitializer& /*initializer*/)
{
	// Do nothing
}

//==============================================================================
void Bs::run()
{
	GlStateSingleton::get().enable(GL_DEPTH_TEST);
	GlStateSingleton::get().setDepthMaskEnabled(false);

	RenderableDrawer& drawer = r->getSceneDrawer();
	drawer.prepareDraw();
	Scene& scene = r->getScene();
	VisibilityInfo& vi =
		scene.getActiveCamera().getFrustumable()->getVisibilityInfo();

	for(auto it = vi.getRenderablesEnd() - 1; it >= vi.getRenderablesBegin();
		--it)
	{
		drawer.render(scene.getActiveCamera(), RenderableDrawer::RS_BLEND,
			0, *(*it));
	}

	GlStateSingleton::get().setDepthMaskEnabled(true);
}

} // end namespace anki
