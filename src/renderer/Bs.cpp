// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
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
	GlCommandBufferHandle cmdb;
	Error err = cmdb.create(&getGlDevice());
	if(err)
	{
		return err;
	}

	err = m_fb.create(
		cmdb,
		{{m_r->getIs()._getRt(), GL_COLOR_ATTACHMENT0}, 
		{m_r->getMs()._getDepthRt(), GL_DEPTH_ATTACHMENT}});

	cmdb.flush();

	return err;
}

//==============================================================================
Error Bs::run(GlCommandBufferHandle& cmdb)
{
	Error err = ErrorCode::NONE;

	m_fb.bind(cmdb, false);

	cmdb.enableDepthTest(true);
	cmdb.setDepthWriteMask(false);
	cmdb.enableBlend(true);

	RenderableDrawer& drawer = m_r->getSceneDrawer();
	drawer.prepareDraw(RenderingStage::BLEND, Pass::COLOR, cmdb);

	Camera& cam = m_r->getSceneGraph().getActiveCamera();
	FrustumComponent& camFr = cam.getComponent<FrustumComponent>();

	auto it = camFr.getVisibilityTestResults().getRenderablesBegin();
	auto end = camFr.getVisibilityTestResults().getRenderablesEnd();
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
