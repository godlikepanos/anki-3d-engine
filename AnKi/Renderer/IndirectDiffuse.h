// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Resource/ImageResource.h>

namespace anki {

ANKI_CVAR(BoolCVar, Render, IndirectDiffuse, true, "Enable the screen-space indirect diffuse pass (requires Idc)")
ANKI_CVAR2(BoolCVar, Render, IndirectDiffuse, InlineRt, true, "Use a cheap and less accurate path with inline RT")
ANKI_CVAR2(BoolCVar, Render, IndirectDiffuse, QuarterRez, true, "Quarter or full resolution")
ANKI_CVAR2(NumericCVar<F32>, Render, IndirectDiffuse, FirstBounceRayDistance, (ANKI_PLATFORM_MOBILE) ? 0.0f : 10.0f, 0.0f, 10000.0f,
		   "For the 1st bounce shoot rays instead of sampling the clipmaps")

// A pass that computes the indirect diffuse of the screen-space visible pixels.
class IndirectDiffuse : public RtMaterialFetchRendererObject
{
public:
	IndirectDiffuse()
	{
		registerDebugRenderTarget("IndirectDiffuse");
	}

	void getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, U32(DebugRenderTargetRegister::kCount)>& handles,
							  [[maybe_unused]] DebugRenderTargetDrawStyle& drawStyle) const override
	{
		handles[0] = m_runCtx.m_rt;
		drawStyle = DebugRenderTargetDrawStyle::kTonemap;
	}

	Error init();

	void populateRenderGraph();

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

private:
	RendererRtShaderProgram m_rtMaterialFetchProg;
	RendererRtShaderProgram m_missProg;
	RendererShaderProgram m_applyProbeIrradianceProg;
	RendererShaderProgram m_applyGiUsingInlineRtProg;
	RendererShaderProgram m_temporalDenoiseProg;
	RendererShaderProgram m_antiFireflyProg;
	RendererShaderProgram m_bilateralDenoiseProg;
	RendererShaderProgram m_upscaleProg;

	RenderTargetDesc m_tmpRtDesc1;
	RenderTargetDesc m_tmpRtDesc2;

	RendererTexture m_finalTex;

	ImageResourcePtr m_blueNoiseImg;

	U32 m_sbtRecordSize = 0;

	Bool m_texturesImportedOnce = false;

	class
	{
	public:
		RenderTargetHandle m_rt;
	} m_runCtx;
};

} // end namespace anki
