#include "anki/renderer/Ez.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/App.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/Camera.h"

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
}

//==============================================================================
void Ez::run()
{
	ANKI_ASSERT(enabled);

	SceneGraph& scene = r->getSceneGraph();
	Camera& cam = scene.getActiveCamera();

	VisibilityTestResults& vi = 
		*cam.getFrustumable()->getVisibilityTestResults();

	U count = 0;
	for(auto it = vi.getRenderablesBegin();
		it != vi.getRenderablesEnd() && count < maxObjectsToDraw; ++it)
	{
		r->getSceneDrawer().render(cam, RenderableDrawer::RS_MATERIAL,
			DEPTH_PASS, *(*it).node, (*it).subSpatialIndices, 
			(*it).subSpatialIndicesCount);
		++count;
	}
}

} // end namespace anki
