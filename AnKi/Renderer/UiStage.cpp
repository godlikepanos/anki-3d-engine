// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/UiStage.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Ui/Font.h>
#include <AnKi/Ui/UiManager.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

UiStage::UiStage(Renderer* r)
	: RendererObject(r)
{
}

UiStage::~UiStage()
{
}

Error UiStage::init()
{
	ANKI_CHECK(m_r->getUiManager().newInstance(m_font, "EngineAssets/UbuntuRegular.ttf", Array<U32, 3>{12, 16, 20}));
	ANKI_CHECK(m_r->getUiManager().newInstance(m_canvas, m_font, 12, m_r->getPostProcessResolution().x(),
											   m_r->getPostProcessResolution().y()));

	return Error::kNone;
}

void UiStage::draw(U32 width, U32 height, RenderingContext& ctx, CommandBufferPtr& cmdb)
{
	// Early exit
	if(ctx.m_renderQueue->m_uis.getSize() == 0)
	{
		return;
	}

	ANKI_TRACE_SCOPED_EVENT(UI_RENDER);

	m_canvas->handleInput();
	m_canvas->beginBuilding();
	m_canvas->resize(width, height);

	for(UiQueueElement& el : ctx.m_renderQueue->m_uis)
	{
		el.m_drawCallback(m_canvas, el.m_userData);
	}

	m_canvas->appendToCommandBuffer(cmdb);

	// UI messes with the state, restore it
	cmdb->setBlendFactors(0, BlendFactor::kOne, BlendFactor::kZero);
	cmdb->setBlendOperation(0, BlendOperation::kAdd);
	cmdb->setCullMode(FaceSelectionBit::kBack);
}

} // end namespace anki
