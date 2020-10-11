// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>
#include <anki/Gr.h>
#include <anki/util/Enum.h>
#include <anki/renderer/RenderQueue.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Debugging stage
class Dbg : public RendererObject
{
public:
	Dbg(Renderer* r);

	~Dbg();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

	Bool getEnabled() const
	{
		return m_enabled;
	}

	void setEnabled(Bool e)
	{
		m_enabled = e;
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
	Bool m_enabled = false;
	Bool m_initialized = false; ///< Lazily initialize.
	RenderTargetDescription m_rtDescr;
	FramebufferDescription m_fbDescr;
	BitSet<U(RenderQueueDebugDrawFlag::COUNT), U32> m_debugDrawFlags = {false};

	class
	{
	public:
		RenderTargetHandle m_rt;
		RenderingContext* m_ctx = nullptr;
	} m_runCtx;

	ANKI_USE_RESULT Error lazyInit();

	void run(RenderPassWorkContext& rgraphCtx, const RenderingContext& ctx);
};
/// @}

} // end namespace anki
