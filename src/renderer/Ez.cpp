// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
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
Error Ez::init(const ConfigSet& config)
{
	m_enabled = config.get("ms.ez.enabled");
	m_maxObjectsToDraw = config.get("ms.ez.maxObjectsToDraw");

	return ErrorCode::NONE;
}

//==============================================================================
Error Ez::run(CommandBufferPtr& cmdBuff)
{
	ANKI_ASSERT(m_enabled);

	Error err = ErrorCode::NONE;
	SceneGraph& scene = m_r->getSceneGraph();
	Camera& cam = scene.getActiveCamera();
	FrustumComponent& camFr = cam.getComponent<FrustumComponent>();

	m_r->getSceneDrawer().prepareDraw(
		RenderingStage::MATERIAL, Pass::DEPTH, cmdBuff);

	U count = m_maxObjectsToDraw;
	auto it = camFr.getVisibilityTestResults().getRenderablesBegin();
	auto end = camFr.getVisibilityTestResults().getRenderablesEnd();
	for(; it != end; ++it)
	{
		err = m_r->getSceneDrawer().render(cam, *it);

		if(--count == 0 || err)
		{
			break;
		}
	}

	if(!err)
	{
		m_r->getSceneDrawer().finishDraw();
	}

	return err;
}

} // end namespace anki
