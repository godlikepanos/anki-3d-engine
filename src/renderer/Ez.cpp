#include "anki/renderer/Ez.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/App.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/Camera.h"

namespace anki {

//==============================================================================
void Ez::init(const RendererInitializer& initializer)
{
	enabled = initializer.get("ms.ez.enabled");
	maxObjectsToDraw = initializer.get("ms.ez.maxObjectsToDraw");
}

//==============================================================================
void Ez::run()
{
	ANKI_ASSERT(enabled);

	SceneGraph& scene = r->getSceneGraph();
	Camera& cam = scene.getActiveCamera();

	VisibilityTestResults& vi = 
		cam.getFrustumComponent()->getVisibilityTestResults();

	U count = 0;
	for(auto it : vi.renderables)
	{
		r->getSceneDrawer().render(cam, RenderableDrawer::RS_MATERIAL,
			DEPTH_PASS, *it.node, it.subSpatialIndices, 
			it.subSpatialIndicesCount);
		++count;
	}
}

} // end namespace anki
