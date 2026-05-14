// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

ANKI_CVAR2(BoolCVar, Render, Reflections, Rt, true, "Enable RT reflections")
ANKI_CVAR2(BoolCVar, Render, Reflections, InlineRt, false, "Enable a cheap inline RT alternative path")
ANKI_CVAR2(NumericCVar<F32>, Render, Reflections, RtMaxRayDistance, 100.0f, 1.0f, 10000.0f, "Max RT reflections ray distance")
ANKI_CVAR2(NumericCVar<U32>, Render, Reflections, SsrStepIncrement, 32, 1, 256, "The number of steps for each loop")
ANKI_CVAR2(NumericCVar<U32>, Render, Reflections, SsrMaxIterations, ANKI_PLATFORM_MOBILE ? 16 : 64, 1, 256, "Max SSR raymarching loop iterations")
ANKI_CVAR2(NumericCVar<F32>, Render, Reflections, RoughnessCutoffToGiEdge0, 0.7f, 0.0f, 1.0f,
		   "Before this roughness the reflections will never sample the GI probes")
ANKI_CVAR2(NumericCVar<F32>, Render, Reflections, RoughnessCutoffToGiEdge1, 0.9f, 0.0f, 1.0f,
		   "After this roughness the reflections will sample the GI probes")
ANKI_CVAR2(BoolCVar, Render, Reflections, QuarterResolution, true, "Render in 1/4 of the resolution")

class Reflections : public RtMaterialFetchRendererObject
{
public:
	Reflections()
	{
		registerDebugRenderTarget("Reflections");
	}

	Error init();

	void populateRenderGraph();

	void getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, U32(DebugRenderTargetRegister::kCount)>& handles,
							  DebugRenderTargetDrawStyle& drawStyle) const override
	{
		handles[0] = m_runCtx.m_rt;
		drawStyle = DebugRenderTargetDrawStyle::kTonemap;
	}

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

public:
	RendererShaderProgram m_tileClassificationProg;
	RendererShaderProgram m_ssrProg;
	RendererShaderProgram m_probeFallbackProg;
	RendererRtShaderProgram m_libraryProg;
	RendererRtShaderProgram m_missProg;
	RendererShaderProgram m_rtMaterialFetchInlineRtProg;
	RendererShaderProgram m_temporalDenoisingProg;
	RendererShaderProgram m_spatialAccumulationProg;
	RendererShaderProgram m_spatialDenoisingProg;
	RendererShaderProgram m_upscalingProg;

	RenderTargetDesc m_classTileMapRtDesc;
	RenderTargetDesc m_colorAndDepth1RtDesc;
	RenderTargetDesc m_colorAndDepth2RtDesc;
	RenderTargetDesc m_hitPosAndPdf1RtDesc;
	RenderTargetDesc m_hitPosAndPdf2RtDesc;

	BufferView m_shaderGroupHandlesBuff;

	// 2 x DispatchIndirectArgs. 1st is for RT and 2nd for probe fallback
	SegregatedListsGpuMemoryPoolAllocation m_indirectArgsBuffer;

	RendererTexture m_tex;
	Array<RendererTexture, 2> m_momentsTex;
	Bool m_texImportedOnce = false;

	U32 m_sbtRecordSize = 0;
	U32 m_rayGenShaderGroupIdx = 0;
	U32 m_missShaderGroupIdx = 0;

	static constexpr U32 kTileSize = 32;

	class
	{
	public:
		RenderTargetHandle m_rt;
	} m_runCtx;
};

} // namespace anki
