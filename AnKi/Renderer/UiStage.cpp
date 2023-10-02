// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/UiStage.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Ui/Font.h>
#include <AnKi/Ui/UiManager.h>
#include <AnKi/Scene/Components/UiComponent.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

Error UiStage::init()
{
	ANKI_CHECK(UiManager::getSingleton().newInstance(m_font, "EngineAssets/UbuntuRegular.ttf", Array<U32, 3>{12, 16, 20}));
	ANKI_CHECK(UiManager::getSingleton().newInstance(m_canvas, m_font, 12, getRenderer().getPostProcessResolution().x(),
													 getRenderer().getPostProcessResolution().y()));

	return Error::kNone;
}

void UiStage::draw(U32 width, U32 height, CommandBuffer& cmdb)
{
	if(SceneGraph::getSingleton().getComponentArrays().getUis().getSize() == 0)
	{
		// Early exit
		return;
	}

	ANKI_TRACE_SCOPED_EVENT(Ui);

	m_canvas->handleInput();
	m_canvas->beginBuilding();
	m_canvas->resize(width, height);

	for(UiComponent& comp : SceneGraph::getSingleton().getComponentArrays().getUis())
	{
		comp.drawUi(m_canvas);
	}

	m_canvas->appendToCommandBuffer(cmdb);

	// UI messes with the state, restore it
	cmdb.setBlendFactors(0, BlendFactor::kOne, BlendFactor::kZero);
	cmdb.setBlendOperation(0, BlendOperation::kAdd);
	cmdb.setCullMode(FaceSelectionBit::kBack);
}

} // end namespace anki
