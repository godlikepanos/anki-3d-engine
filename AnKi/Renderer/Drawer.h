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

/// @addtogroup renderer
/// @{

/// @memberof RenderableDrawer.
class RenderableDrawerArguments
{
public:
	// The matrices are whatever the drawing needs. Sometimes they contain jittering and sometimes they don't.
	Mat3x4 m_viewMatrix;
	Mat3x4 m_cameraTransform;
	Mat4 m_viewProjectionMatrix;
	Mat4 m_previousViewProjectionMatrix;

	SamplerPtr m_sampler;
	U32 m_minLod = 0;
	U32 m_maxLod = kMaxLodCount - 1;
};

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

	void drawRange(RenderingTechnique technique, const RenderableDrawerArguments& args,
				   const RenderableQueueElement* begin, const RenderableQueueElement* end, CommandBufferPtr& cmdb);

private:
	class Context;

	Renderer* m_r;

	void flushDrawcall(Context& ctx);

	void drawSingle(Context& ctx);
};
/// @}

} // end namespace anki
