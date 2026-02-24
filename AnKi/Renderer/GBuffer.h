// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Utils/GpuVisibility.h>
#include <AnKi/Gr.h>

namespace anki {

ANKI_CVAR(BoolCVar, Render, VisualizeGiProbes, false, "Visualize GI probes")
ANKI_CVAR(BoolCVar, Render, VisualizeReflectionProbes, false, "Visualize reflection probes")

// G buffer stage. It populates the G buffer
class GBuffer : public RendererObject
{
public:
	GBuffer()
	{
		registerDebugRenderTarget("GBufferNormals");
		registerDebugRenderTarget("GBufferAlbedo");
		registerDebugRenderTarget("GBufferVelocity");
		registerDebugRenderTarget("GBufferRoughness");
		registerDebugRenderTarget("GBufferMetallic");
		registerDebugRenderTarget("GBufferSubsurface");
		registerDebugRenderTarget("GBufferEmission");
	}

	~GBuffer();

	Error init();

	void importRenderTargets();

	void populateRenderGraph();

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

	const RenderTargetHandle& getHzbRt() const
	{
		return m_runCtx.m_hzbRt;
	}

	void getDebugRenderTarget(CString rtName, Array<RenderTargetHandle, U32(DebugRenderTargetRegister::kCount)>& handles,
							  DebugRenderTargetDrawStyle& drawStyle) const override;

	// Returns a buffer with indices of the visible AABBs. Used in debug drawing.
	const GpuVisibilityOutput& getVisibilityOutput() const
	{
		return m_runCtx.m_visOut;
	}

private:
	Array<RenderTargetDesc, kGBufferColorRenderTargetCount> m_colorRtDescrs;
	Array<RendererTexture, 2> m_depthRts;
	RendererTexture m_hzbRt;

	ShaderProgramResourcePtr m_visualizeProbeProg;
	ShaderProgramPtr m_visualizeGiProbeGrProg;
	ShaderProgramPtr m_visualizeReflProbeGrProg;

	class
	{
	public:
		Array<RenderTargetHandle, kGBufferColorRenderTargetCount> m_colorRts;
		RenderTargetHandle m_crntFrameDepthRt;
		RenderTargetHandle m_prevFrameDepthRt;
		RenderTargetHandle m_hzbRt;

		GpuVisibilityOutput m_visOut;
	} m_runCtx;
};

} // end namespace anki
