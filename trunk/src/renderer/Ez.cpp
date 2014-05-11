#include "anki/renderer/Ez.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/App.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/Camera.h"

namespace anki {

//==============================================================================
void Ez::init(const RendererInitializer& initializer)
{
	m_enabled = initializer.get("ms.ez.enabled");
	m_maxObjectsToDraw = initializer.get("ms.ez.maxObjectsToDraw");
}

//==============================================================================
void Ez::run(GlJobChainHandle& jobs)
{
	ANKI_ASSERT(m_enabled);

	SceneGraph& scene = m_r->getSceneGraph();
	Camera& cam = scene.getActiveCamera();

	VisibilityTestResults& vi = cam.getVisibilityTestResults();

	m_r->getSceneDrawer().prepareDraw(
		RenderingStage::MATERIAL, Pass::DEPTH, jobs);

	U count = m_maxObjectsToDraw;
	for(auto& it : vi.m_renderables)
	{
		m_r->getSceneDrawer().render(cam, it);

		if(--count == 0)
		{
			break;
		}
	}

	m_r->getSceneDrawer().finishDraw();
}

} // end namespace anki
