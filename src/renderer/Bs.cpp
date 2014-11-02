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
Error Bs::init(const ConfigSet&)
{
	return ErrorCode::NONE;
}

//==============================================================================
Error Bs::run(GlCommandBufferHandle& cmdb)
{
	Error err = ErrorCode::NONE;

	cmdb.enableDepthTest(true);
	cmdb.setDepthWriteMask(false);
	cmdb.enableBlend(true);

	RenderableDrawer& drawer = m_r->getSceneDrawer();
	drawer.prepareDraw(RenderingStage::BLEND, Pass::COLOR, cmdb);

	Camera& cam = m_r->getSceneGraph().getActiveCamera();

	auto it = cam.getVisibilityTestResults().getRenderablesBegin();
	auto end = cam.getVisibilityTestResults().getRenderablesEnd();
	for(; !err && it != end; ++it)
	{
		err = drawer.render(cam, *it);
	}

	if(!err)
	{
		drawer.finishDraw();

		cmdb.enableDepthTest(false);
		cmdb.setDepthWriteMask(true);
		cmdb.enableBlend(false);
	}

	return err;
}

} // end namespace anki
