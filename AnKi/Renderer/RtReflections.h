// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

inline BoolCVar g_rtReflectionsCVar("R", "RtReflections", false, "Enable RT reflections");
inline NumericCVar<F32> g_rtReflectionsMaxRayDistanceCVar("R", "RtReflectionsMaxRayDistance", 100.0f, 1.0f, 10000.0f,
														  "Max RT reflections ray distance");

class RtReflections : public RendererObject
{
public:
	RtReflections()
	{
		registerDebugRenderTarget("RtReflections");
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
	ShaderProgramResourcePtr m_rtProg;
	ShaderProgramPtr m_sbtBuildSetupGrProg;
	ShaderProgramPtr m_sbtBuildGrProg;
	ShaderProgramPtr m_libraryGrProg;
	ShaderProgramPtr m_spatialDenoisingGrProg;
	ShaderProgramPtr m_temporalDenoisingGrProg;
	ShaderProgramPtr m_verticalBilateralDenoisingGrProg;
	ShaderProgramPtr m_horizontalBilateralDenoisingGrProg;

	RenderTargetDesc m_transientRtDesc1;
	RenderTargetDesc m_transientRtDesc2;
	RenderTargetDesc m_hitPosAndDepthRtDesc;

	TexturePtr m_tex;
	Array<TexturePtr, 2> m_momentsTextures;
	Bool m_texImportedOnce = false;

	U32 m_sbtRecordSize = 0;
	U32 m_rayGenShaderGroupIdx = 0;
	U32 m_missShaderGroupIdx = 0;

	class
	{
	public:
		RenderTargetHandle m_rt;
	} m_runCtx;
};
/// @}

} // namespace anki
