// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

inline BoolCVar g_rtMaterialFetchDbgCVar("R", "RtMaterialFetchDbg", false, "Enable material debugging pass");

/// Similar to ShadowmapsResolve but it's using ray tracing.
class RtMaterialFetchDbg : public RendererObject
{
public:
	RtMaterialFetchDbg()
	{
		registerDebugRenderTarget("RtMaterialFetchDbg");
	}

	Error init();

	void populateRenderGraph(RenderingContext& ctx);

	void getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
							  [[maybe_unused]] ShaderProgramPtr& optionalShaderProgram) const override
	{
		handles[0] = m_runCtx.m_rt;
	}

public:
	ShaderProgramResourcePtr m_sbtProg;
	ShaderProgramResourcePtr m_rtProg;
	ShaderProgramResourcePtr m_missProg;
	ShaderProgramPtr m_sbtBuildGrProg;
	ShaderProgramPtr m_libraryGrProg;

	RenderTargetDesc m_rtDesc;

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
