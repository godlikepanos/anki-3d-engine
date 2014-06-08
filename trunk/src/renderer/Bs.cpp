// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Bs.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/Camera.h"

namespace anki {

//==============================================================================
Bs::~Bs()
{}

//==============================================================================
void Bs::init(const ConfigSet&)
{
	// Do nothing
}

//==============================================================================
void Bs::run(GlJobChainHandle& jobs)
{
	jobs.enableDepthTest(true);
	jobs.setDepthWriteMask(false);
	jobs.enableBlend(true);

	RenderableDrawer& drawer = m_r->getSceneDrawer();
	drawer.prepareDraw(RenderingStage::BLEND, Pass::COLOR, jobs);

	Camera& cam = m_r->getSceneGraph().getActiveCamera();

	for(auto& it : cam.getVisibilityTestResults().m_renderables)
	{
		drawer.render(cam, it);
	}

	drawer.finishDraw();

	jobs.enableDepthTest(false);
	jobs.setDepthWriteMask(true);
	jobs.enableBlend(false);
}

} // end namespace anki
