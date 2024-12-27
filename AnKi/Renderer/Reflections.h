// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

inline BoolCVar g_rtReflectionsCVar("R", "RtReflections", true, "Enable RT reflections");
inline NumericCVar<F32> g_rtReflectionsMaxRayDistanceCVar("R", "RtReflectionsMaxRayDistance", 100.0f, 1.0f, 10000.0f,
														  "Max RT reflections ray distance");
inline NumericCVar<U32> g_ssrStepIncrementCVar("R", "SsrStepIncrement", 32, 1, 256, "The number of steps for each loop");
inline NumericCVar<U32> g_ssrMaxIterationsCVar("R", "SsrMaxIterations", 64, 1, 256, "Max SSR raymarching loop iterations");

inline NumericCVar<F32> g_roughnessCutoffToGiEdge0("R", "RoughnessCutoffToGiEdge0", 0.7f, 0.0f, 1.0f,
												   "Before this roughness the reflections will never sample the GI probes");
inline NumericCVar<F32> g_roughnessCutoffToGiEdge1("R", "RoughnessCutoffToGiEdge1", 0.9f, 0.0f, 1.0f,
												   "After this roughness the reflections will sample the GI probes");

class Reflections : public RendererObject
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
	ShaderProgramResourcePtr m_sbtProg;
	ShaderProgramResourcePtr m_mainProg;
	ShaderProgramPtr m_ssrGrProg;
	ShaderProgramPtr m_sbtBuildGrProg;
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
