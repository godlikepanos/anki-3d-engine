// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>
#include <anki/Gr.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// G buffer stage. It populates the G buffer
class GBuffer : public RendererObject
{
public:
	GBuffer(Renderer* r)
		: RendererObject(r)
	{
		registerDebugRenderTarget("GBuffer_normals");
		registerDebugRenderTarget("GBuffer_albedo");
		registerDebugRenderTarget("GBuffer_velocity");
	}

	~GBuffer();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getColorRt(U idx) const
	{
		return m_runCtx.m_colorRts[idx];
	}

	RenderTargetHandle getDepthRt() const
	{
		return m_runCtx.m_crntFrameDepthRt;
	}

	RenderTargetHandle getPreviousFrameDepthRt() const
	{
		return m_runCtx.m_prevFrameDepthRt;
	}

	void getDebugRenderTarget(CString rtName, RenderTargetHandle& handle) const override
	{
		if(rtName == "GBuffer_albedo")
		{
			handle = m_runCtx.m_colorRts[0];
		}
		else if(rtName == "GBuffer_normals")
		{
			handle = m_runCtx.m_colorRts[2];
		}
		else if(rtName == "GBuffer_velocity")
		{
			handle = m_runCtx.m_colorRts[3];
		}
		else
		{
			ANKI_ASSERT(!"See file");
		}
	}

private:
	Array<RenderTargetDescription, GBUFFER_COLOR_ATTACHMENT_COUNT> m_colorRtDescrs;
	Array<TexturePtr, 2> m_depthRts;
	FramebufferDescription m_fbDescr;

	class
	{
	public:
		RenderingContext* m_ctx = nullptr;

		Array<RenderTargetHandle, GBUFFER_COLOR_ATTACHMENT_COUNT> m_colorRts;
		RenderTargetHandle m_crntFrameDepthRt;
		RenderTargetHandle m_prevFrameDepthRt;
	} m_runCtx;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);

	void runInThread(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx) const;
};
/// @}

} // end namespace anki
