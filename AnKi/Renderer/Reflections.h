// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

ANKI_CVAR2(BoolCVar, Render, Reflections, Rt, true, "Enable RT reflections")
ANKI_CVAR2(NumericCVar<F32>, Render, Reflections, RtMaxRayDistance, 100.0f, 1.0f, 10000.0f, "Max RT reflections ray distance")
ANKI_CVAR2(NumericCVar<U32>, Render, Reflections, SsrStepIncrement, 32, 1, 256, "The number of steps for each loop")
ANKI_CVAR2(NumericCVar<U32>, Render, Reflections, SsrMaxIterations, ANKI_PLATFORM_MOBILE ? 16 : 64, 1, 256, "Max SSR raymarching loop iterations")

ANKI_CVAR2(NumericCVar<F32>, Render, Reflections, RoughnessCutoffToGiEdge0, 0.7f, 0.0f, 1.0f,
		   "Before this roughness the reflections will never sample the GI probes")
ANKI_CVAR2(NumericCVar<F32>, Render, Reflections, RoughnessCutoffToGiEdge1, 0.9f, 0.0f, 1.0f,
		   "After this roughness the reflections will sample the GI probes")

class Reflections : public RtMaterialFetchRendererObject
{
public:
	Reflections()
	{
		registerDebugRenderTarget("Reflections");
	}

	Error init();

	void populateRenderGraph(RenderingContext& ctx);

	void getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
							  [[maybe_unused]] ShaderProgramPtr& optionalShaderProgram) const override
	{
		handles[0] = m_runCtx.m_rt;
	}

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

public:
	ShaderProgramResourcePtr m_mainProg;
	ShaderProgramResourcePtr m_missProg;
	ShaderProgramPtr m_ssrGrProg;
	ShaderProgramPtr m_libraryGrProg;
	ShaderProgramPtr m_spatialDenoisingGrProg;
	ShaderProgramPtr m_temporalDenoisingGrProg;
	ShaderProgramPtr m_verticalBilateralDenoisingGrProg;
	ShaderProgramPtr m_horizontalBilateralDenoisingGrProg;
	ShaderProgramPtr m_probeFallbackGrProg;
	ShaderProgramPtr m_tileClassificationGrProg;

	RenderTargetDesc m_transientRtDesc1;
	RenderTargetDesc m_transientRtDesc2;
	RenderTargetDesc m_hitPosAndDepthRtDesc;
	RenderTargetDesc m_hitPosRtDesc;
	RenderTargetDesc m_classTileMapRtDesc;

	/// 2 x DispatchIndirectArgs. 1st is for RT and 2nd for probe fallback
	BufferPtr m_indirectArgsBuffer;

	TexturePtr m_tex;
	Array<TexturePtr, 2> m_momentsTextures;
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
/// @}

} // namespace anki
