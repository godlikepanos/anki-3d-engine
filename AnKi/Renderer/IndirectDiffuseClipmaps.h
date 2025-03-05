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
inline NumericCVar<U32> g_indirectDiffuseClipmap0ProbesPerDimCVar("R", "IndirectDiffuseClipmap0ProbesPerDim", 40, 10, 50,
																  "The cell count of each dimension of 1st clipmap");
inline NumericCVar<F32> g_indirectDiffuseClipmap0SizeCVar("R", "IndirectDiffuseClipmap0Size", 20.0, 10.0, 100.0, "The clipmap size in meters");

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

private:
	static constexpr U32 kClipmapLevelCount = 3;

	Array<TexturePtr, kClipmapLevelCount> m_clipmapLevelTextures;

	ShaderProgramResourcePtr m_tmpProg;
	ShaderProgramPtr m_tmpGrProg;
	ShaderProgramPtr m_tmpGrProg2;

	RenderTargetDesc m_tmpRtDesc;

	Bool m_clipmapsImportedOnce = false;

	class
	{
	public:
		RenderTargetHandle m_tmpRt;
	} m_runCtx;
};
/// @}

} // end namespace anki
