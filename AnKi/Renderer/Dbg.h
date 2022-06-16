// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Gr.h>
#include <AnKi/Util/Enum.h>
#include <AnKi/Renderer/RenderQueue.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Debugging stage
class Dbg : public RendererObject
{
public:
	Dbg(Renderer* r);

	~Dbg();

	Error init();

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

	Bool getDepthTestEnabled() const
	{
		return m_debugDrawFlags.get(RenderQueueDebugDrawFlag::DEPTH_TEST_ON);
	}

	void setDepthTestEnabled(Bool enable)
	{
		m_debugDrawFlags.set(RenderQueueDebugDrawFlag::DEPTH_TEST_ON, enable);
	}

	void switchDepthTestEnabled()
	{
		m_debugDrawFlags.flip(RenderQueueDebugDrawFlag::DEPTH_TEST_ON);
	}

	Bool getDitheredDepthTestEnabled() const
	{
		return m_debugDrawFlags.get(RenderQueueDebugDrawFlag::DITHERED_DEPTH_TEST_ON);
	}

	void setDitheredDepthTestEnabled(Bool enable)
	{
		m_debugDrawFlags.set(RenderQueueDebugDrawFlag::DITHERED_DEPTH_TEST_ON, enable);
	}

	void switchDitheredDepthTestEnabled()
	{
		m_debugDrawFlags.flip(RenderQueueDebugDrawFlag::DITHERED_DEPTH_TEST_ON);
	}

private:
	RenderTargetDescription m_rtDescr;
	FramebufferDescription m_fbDescr;
	BitSet<U(RenderQueueDebugDrawFlag::COUNT), U32> m_debugDrawFlags = {false};

	class
	{
	public:
		RenderTargetHandle m_rt;
	} m_runCtx;

	void run(RenderPassWorkContext& rgraphCtx, const RenderingContext& ctx);
};
/// @}

} // end namespace anki
