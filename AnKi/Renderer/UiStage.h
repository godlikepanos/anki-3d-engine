// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Ui/Canvas.h>
#include <AnKi/Ui/Font.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Ui pass.
class UiStage : public RendererObject
{
public:
	Error init();

	void draw(U32 width, U32 height, RenderingContext& ctx, CommandBufferPtr& cmdb);

private:
	FontPtr m_font;
	CanvasPtr m_canvas;
};
/// @}

} // end namespace anki
