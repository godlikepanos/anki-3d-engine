// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Utils/GpuVisibility.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Forward rendering stage. The objects that blend must be handled differently
class ForwardShading : public RendererObject
{
public:
	ForwardShading() = default;

	~ForwardShading() = default;

	Error init()
	{
		return Error::kNone;
	}

	void populateRenderGraph(RenderingContext& ctx);

	void setDependencies(GraphicsRenderPassDescription& pass);

	void run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);

	/// Returns a buffer with indices of the visible AABBs. Used in debug drawing.
	void getVisibleAabbsBuffer(BufferOffsetRange& visibleAaabbIndicesBuffer, BufferHandle& dep) const
	{
		visibleAaabbIndicesBuffer = m_runCtx.m_visOut.m_visibleAaabbIndicesBuffer;
		dep = m_runCtx.m_visOut.m_someBufferHandle;
		ANKI_ASSERT(visibleAaabbIndicesBuffer.m_buffer != nullptr && dep.isValid());
	}

private:
	class
	{
	public:
		GpuVisibilityOutput m_visOut;
	} m_runCtx;
};
/// @}

} // end namespace anki
