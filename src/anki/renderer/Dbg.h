// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>
#include <anki/Gr.h>
#include <anki/util/Enum.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Debugging stage
class Dbg : public RendererObject
{
public:
	Bool getEnabled() const
	{
		return m_enabled;
	}

	void setEnabled(Bool e)
	{
		m_enabled = e;
	}

	Bool getDepthTestEnabled() const;
	void setDepthTestEnabled(Bool enable);
	void switchDepthTestEnabled();

anki_internal:
	Dbg(Renderer* r);

	~Dbg();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

private:
	Bool8 m_enabled = false;
	Bool8 m_initialized = false; ///< Lazily initialize.
	RenderTargetDescription m_rtDescr;
	FramebufferDescription m_fbDescr;
	DebugDrawer* m_drawer = nullptr;

	class
	{
	public:
		RenderTargetHandle m_rt;
		RenderingContext* m_ctx = nullptr;
	} m_runCtx;

	ANKI_USE_RESULT Error lazyInit();

	// A RenderPassWorkCallback for debug pass.
	static void runCallback(RenderPassWorkContext& rgraphCtx)
	{
		Dbg* self = static_cast<Dbg*>(rgraphCtx.m_userData);
		self->run(rgraphCtx, *self->m_runCtx.m_ctx);
	}

	void run(RenderPassWorkContext& rgraphCtx, const RenderingContext& ctx);
};
/// @}

} // end namespace anki
