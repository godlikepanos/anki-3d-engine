// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/HzbHelper.h>
#include <AnKi/Gr.h>

namespace anki {

// Forward
class GpuVisibilityOutput;

/// @addtogroup renderer
/// @{

/// G buffer stage. It populates the G buffer
class GBuffer : public RendererObject
{
public:
	GBuffer()
	{
		registerDebugRenderTarget("GBufferNormals");
		registerDebugRenderTarget("GBufferAlbedo");
		registerDebugRenderTarget("GBufferVelocity");
	}

	~GBuffer();

	Error init();

	void importRenderTargets(RenderingContext& ctx);

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

	void getDebugRenderTarget(CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
							  [[maybe_unused]] ShaderProgramPtr& optionalShaderProgram) const override
	{
		if(rtName == "GBufferAlbedo")
		{
			handles[0] = m_runCtx.m_colorRts[0];
		}
		else if(rtName == "GBufferNormals")
		{
			handles[0] = m_runCtx.m_colorRts[2];
		}
		else if(rtName == "GBufferVelocity")
		{
			handles[0] = m_runCtx.m_colorRts[3];
		}
		else
		{
			ANKI_ASSERT(!"See file");
		}
	}

private:
	Array<RenderTargetDescription, kGBufferColorRenderTargetCount> m_colorRtDescrs;
	Array<TexturePtr, 2> m_depthRts;
	TexturePtr m_hzbRt;
	FramebufferDescription m_fbDescr;

	HzbHelper m_hzb;

	class
	{
	public:
		Array<RenderTargetHandle, kGBufferColorRenderTargetCount> m_colorRts;
		RenderTargetHandle m_crntFrameDepthRt;
		RenderTargetHandle m_prevFrameDepthRt;
		RenderTargetHandle m_hzbRt;
	} m_runCtx;

	Error initInternal();

	void runInThread(const RenderingContext& ctx, const GpuVisibilityOutput& visOut, RenderPassWorkContext& rgraphCtx) const;
};
/// @}

} // end namespace anki
