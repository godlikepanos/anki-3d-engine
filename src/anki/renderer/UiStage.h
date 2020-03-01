// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>
#include <anki/ui/Canvas.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Ui pass.
class UiStage : public RendererObject
{
public:
	UiStage(Renderer* r);
	~UiStage();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	void draw(U32 width, U32 height, RenderingContext& ctx, CommandBufferPtr& cmdb);

private:
	FontPtr m_font;
	CanvasPtr m_canvas;
};
/// @}

} // end namespace anki
