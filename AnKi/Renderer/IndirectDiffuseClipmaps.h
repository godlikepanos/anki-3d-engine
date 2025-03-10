// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Utils/TraditionalDeferredShading.h>
#include <AnKi/Collision/Forward.h>

namespace anki {

/// @addtogroup renderer
/// @{

inline BoolCVar g_rtIndirectDiffuseClipmapsCVar("R", "RtIndirectDiffuseClipmaps", false);

inline NumericCVar<U32> g_indirectDiffuseClipmap0ProbesPerDimCVar("R", "IndirectDiffuseClipmap0ProbesPerDim", 20, 10, 50,
																  "The cell count of each dimension of 1st clipmap");
inline NumericCVar<U32> g_indirectDiffuseClipmap1ProbesPerDimCVar("R", "IndirectDiffuseClipmap1ProbesPerDim", 20, 10, 50,
																  "The cell count of each dimension of 2nd clipmap");
inline NumericCVar<U32> g_indirectDiffuseClipmap2ProbesPerDimCVar("R", "IndirectDiffuseClipmap2ProbesPerDim", 20, 10, 50,
																  "The cell count of each dimension of 3rd clipmap");

inline NumericCVar<F32> g_indirectDiffuseClipmap0SizeCVar("R", "IndirectDiffuseClipmap0Size", 20.0, 10.0, 1000.0, "The clipmap size in meters");
inline NumericCVar<F32> g_indirectDiffuseClipmap1SizeCVar("R", "IndirectDiffuseClipmap1Size", 40.0, 10.0, 1000.0, "The clipmap size in meters");
inline NumericCVar<F32> g_indirectDiffuseClipmap2SizeCVar("R", "IndirectDiffuseClipmap2Size", 80.0, 10.0, 1000.0, "The clipmap size in meters");

/// Ambient global illumination passes.
class IndirectDiffuseClipmaps : public RendererObject
{
public:
	IndirectDiffuseClipmaps()
	{
		registerDebugRenderTarget("IndirectDiffuseClipmapsTest");
	}

	Error init();

	void populateRenderGraph(RenderingContext& ctx);

	void getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
							  [[maybe_unused]] ShaderProgramPtr& optionalShaderProgram) const override
	{
		handles[0] = m_runCtx.m_tmpRt;
	}

	const Array<IndirectDiffuseClipmap, kIndirectDiffuseClipmapCount>& getClipmapsInfo() const
	{
		return m_clipmapInfo;
	}

private:
	class ClipmapVolumes
	{
	public:
		Array<TexturePtr, 3> m_perColorComponent;
	};

	Array<ClipmapVolumes, kIndirectDiffuseClipmapCount> m_clipmapVolumes;

	Array<IndirectDiffuseClipmap, kIndirectDiffuseClipmapCount> m_clipmapInfo;

	ShaderProgramResourcePtr m_prog;
	ShaderProgramResourcePtr m_missProg;
	ShaderProgramResourcePtr m_sbtProg;
	ShaderProgramPtr m_libraryGrProg;
	ShaderProgramPtr m_tmpVisGrProg;
	ShaderProgramPtr m_sbtBuildGrProg;

	RenderTargetDesc m_tmpRtDesc;

	U32 m_sbtRecordSize = 0;
	U32 m_rayGenShaderGroupIdx = kMaxU32;
	U32 m_missShaderGroupIdx = kMaxU32;

	Bool m_clipmapsImportedOnce = false;

	class
	{
	public:
		RenderTargetHandle m_tmpRt;
	} m_runCtx;
};
/// @}

} // end namespace anki
