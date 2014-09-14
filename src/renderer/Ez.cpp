// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Ez.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/App.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/Camera.h"
#include "anki/misc/ConfigSet.h"

namespace anki {

//==============================================================================
void Ez::init(const ConfigSet& config)
{
	m_enabled = config.get("ms.ez.enabled");
	m_maxObjectsToDraw = config.get("ms.ez.maxObjectsToDraw");
}

//==============================================================================
void Ez::run(GlCommandBufferHandle& cmdBuff)
{
	ANKI_ASSERT(m_enabled);

	SceneGraph& scene = m_r->getSceneGraph();
	Camera& cam = scene.getActiveCamera();

	VisibilityTestResults& vi = cam.getVisibilityTestResults();

	m_r->getSceneDrawer().prepareDraw(
		RenderingStage::MATERIAL, Pass::DEPTH, cmdBuff);

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
