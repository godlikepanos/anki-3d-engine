// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Common.h>
#include <anki/resource/RenderingKey.h>
#include <anki/Gr.h>

namespace anki
{

// Forward
class Renderer;
class DrawContext;

/// @addtogroup renderer
/// @{

/// It uses the render queue to batch and render.
class RenderableDrawer
{
	friend class RenderTask;

public:
	RenderableDrawer(Renderer* r)
		: m_r(r)
	{
	}

	~RenderableDrawer();

	void drawRange(Pass pass,
		const Mat4& viewMat,
		const Mat4& viewProjMat,
		const Mat4& prevViewProjMat,
		CommandBufferPtr cmdb,
		const RenderableQueueElement* begin,
		const RenderableQueueElement* end);

private:
	Renderer* m_r;

	void flushDrawcall(DrawContext& ctx);

	void drawSingle(DrawContext& ctx);
};
/// @}

} // end namespace anki
