// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/UiStage.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/ui/Font.h>
#include <anki/ui/UiManager.h>
#include <anki/util/Tracer.h>

namespace anki
{

UiStage::UiStage(Renderer* r)
	: RendererObject(r)
{
}

UiStage::~UiStage()
{
}

Error UiStage::init(const ConfigSet&)
{
	ANKI_CHECK(m_r->getUiManager().newInstance(m_font, "engine_data/UbuntuRegular.ttf",
											   std::initializer_list<U32>{12, 16, 20}));
	ANKI_CHECK(m_r->getUiManager().newInstance(m_canvas, m_font, 12, m_r->getWidth(), m_r->getHeight()));

	return Error::NONE;
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
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
	cmdb->setBlendOperation(0, BlendOperation::ADD);
	cmdb->setCullMode(FaceSelectionBit::BACK);
}

} // end namespace anki
