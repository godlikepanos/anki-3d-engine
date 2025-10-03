// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Ui/UiCanvas.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Ui pass.
class UiStage : public RendererObject
{
public:
	Error init();

	/// Need to wait the CoreThreadJobManager for that to finish and before calling the methods bellow.
	void buildUiAsync();

	void populateRenderGraph(RenderingContext& ctx);

	void setDependencies(RenderPassBase& pass);

	void drawUi(CommandBuffer& cmdb);

private:
	UiCanvasPtr m_canvas;

	class
	{
	public:
		WeakArray<RenderTargetHandle> m_handles;
	} m_runCtx;
};
/// @}

} // end namespace anki
