// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/Common.h>
#include <AnKi/Resource/RenderingKey.h>
#include <AnKi/Gr.h>

namespace anki {

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

	void drawRange(RenderingTechnique technique, const Mat4& viewMat, const Mat4& viewProjMat,
				   const Mat4& prevViewProjMat, CommandBufferPtr cmdb, SamplerPtr sampler,
				   const RenderableQueueElement* begin, const RenderableQueueElement* end, U32 minLod = 0,
				   U32 maxLod = MAX_LOD_COUNT - 1);

private:
	Renderer* m_r;

	void flushDrawcall(DrawContext& ctx);

	void drawSingle(DrawContext& ctx);
};
/// @}

} // end namespace anki
