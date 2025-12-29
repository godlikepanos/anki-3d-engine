// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/UiStage.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Ui/UiManager.h>
#include <AnKi/Scene/Components/UiComponent.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

Error UiStage::init()
{
	ANKI_CHECK(UiManager::getSingleton().newCanvas(getRenderer().getPostProcessResolution().x, getRenderer().getPostProcessResolution().y, m_canvas));

	return Error::kNone;
}

void UiStage::buildUi()
{
	if(SceneGraph::getSingleton().getComponentArrays().getUis().getSize() == 0)
	{
		// Early exit
		return;
	}

	ANKI_TRACE_SCOPED_EVENT(UiBuild);

	if(SceneGraph::getSingleton().isPaused())
	{
		m_canvas->handleInput();
	}
	m_canvas->beginBuilding();
	m_canvas->resize(getRenderer().getSwapchainResolution());

	for(UiComponent& comp : SceneGraph::getSingleton().getComponentArrays().getUis())
	{
		comp.drawUi(*m_canvas);
	}

	m_canvas->endBuilding();
}

void UiStage::populateRenderGraph()
{
	// Create a pass that uploads UI textures

	RenderGraphBuilder& rgraph = getRenderingContext().m_renderGraphDescr;
	DynamicArray<RenderTargetHandle, MemoryPoolPtrWrapper<StackMemoryPool>> texHandles(&getRenderer().getFrameMemoryPool());

	m_canvas->visitTexturesForUpdate([&](Texture& tex, Bool isNew) {
		const RenderTargetHandle handle = rgraph.importRenderTarget(&tex, (isNew) ? TextureUsageBit::kNone : TextureUsageBit::kSrvPixel);
		texHandles.emplaceBack(handle);
	});

	if(texHandles.getSize() == 0) [[likely]]
	{
		m_runCtx.m_handles = {};
		return;
	}

	texHandles.moveAndReset(m_runCtx.m_handles);

	NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("UI copies");

	for(RenderTargetHandle handle : m_runCtx.m_handles)
	{
		pass.newTextureDependency(handle, TextureUsageBit::kCopyDestination);
	}

	pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
		ANKI_TRACE_SCOPED_EVENT(UiCopies);
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;
		m_canvas->appendNonGraphicsCommands(cmdb);
	});
}

void UiStage::setDependencies(RenderPassBase& pass)
{
	for(RenderTargetHandle handle : m_runCtx.m_handles)
	{
		pass.newTextureDependency(handle, TextureUsageBit::kSrvPixel);
	}
}

void UiStage::drawUi(CommandBuffer& cmdb)
{
	if(SceneGraph::getSingleton().getComponentArrays().getUis().getSize() == 0)
	{
		// Early exit
		return;
	}

	ANKI_TRACE_SCOPED_EVENT(Ui);
	m_canvas->appendGraphicsCommands(cmdb);
}

} // end namespace anki
