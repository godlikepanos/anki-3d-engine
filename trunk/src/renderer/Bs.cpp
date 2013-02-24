#include "anki/renderer/Bs.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/SceneGraph.h"
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
	SceneGraph& scene = r->getSceneGraph();
	VisibilityTestResults& vi =
		*scene.getActiveCamera().getFrustumable()->getVisibilityTestResults();

	for(auto it = vi.getRenderablesBegin(); it != vi.getRenderablesEnd();
		++it)
	{
		drawer.render(scene.getActiveCamera(), RenderableDrawer::RS_BLEND,
			0, *(*it));
	}

	GlStateSingleton::get().setDepthMaskEnabled(true);
}

} // end namespace anki
