// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Gr.h>

namespace anki {

/// @addtogroup renderer
/// @{

ANKI_CVAR2(NumericCVar<U32>, Render, Ssao, SampleCount, 4, 1, 1024, "SSAO sample count")
ANKI_CVAR2(NumericCVar<F32>, Render, Ssao, Radius, 2.0f, 0.1f, 100.0f, "SSAO radius in meters")
ANKI_CVAR2(BoolCVar, Render, Ssao, QuarterRez, ANKI_PLATFORM_MOBILE, "Render SSAO in quarter rez")
ANKI_CVAR2(NumericCVar<F32>, Render, Ssao, Power, 1.5f, 0.1f, 100.0f, "SSAO power")
ANKI_CVAR2(
	NumericCVar<U8>, Render, Ssao, SpatialDenoiseSampleCout, (ANKI_PLATFORM_MOBILE) ? 3 : 9,
	[](U8 val) {
		return val == 3 || val == 5 || val == 7 || val == 9;
	},
	"SSAO spatial denoise quality")

/// Screen space ambient occlusion.
class Ssao : public RendererObject
{
public:
	Ssao()
	{
		registerDebugRenderTarget("BentNormals");
		registerDebugRenderTarget("Ssao");
	}

	Error init();

	void populateRenderGraph(RenderingContext& ctx);

	void getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
							  [[maybe_unused]] Array<DebugRenderTargetDrawStyle, kMaxDebugRenderTargets>& drawStyles) const override
	{
		handles[0] = m_runCtx.m_finalRt;
		drawStyles[0] = (rtName == "Ssao") ? DebugRenderTargetDrawStyle::kAlphaOnly : DebugRenderTargetDrawStyle::kPassthrough;
	}

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_finalRt;
	}

public:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;
	ShaderProgramPtr m_spatialDenoiseVerticalGrProg;
	ShaderProgramPtr m_spatialDenoiseHorizontalGrProg;
	ShaderProgramPtr m_tempralDenoiseGrProg;

	RenderTargetDesc m_bentNormalsAndSsaoRtDescr;

	Array<TexturePtr, 2> m_tex;
	Bool m_texImportedOnce = false;

	ImageResourcePtr m_noiseImage;

	class
	{
	public:
		RenderTargetHandle m_finalRt;
	} m_runCtx;

	Error initInternal();
};
/// @}

} // end namespace anki
