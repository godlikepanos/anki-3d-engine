// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

ANKI_CVAR2(NumericCVar<F32>, Render, Bloom, Threshold, 2.5f, 0.0f, 256.0f, "Bloom threshold")
ANKI_CVAR2(NumericCVar<F32>, Render, Bloom, Scale, 2.5f, 0.0f, 256.0f, "Bloom scale")
ANKI_CVAR2(NumericCVar<U32>, Render, Bloom, PyramidLowLimit, 32, 8, 1024, "Downscale the boom pyramid up to that size")
ANKI_CVAR2(NumericCVar<U32>, Render, Bloom, UpscaleDivisor, 4, 1, 1024, "Defines the resolution of the final bloom result")

/// Contains multiple post-process passes that operate on the HDR output.
class Bloom : public RendererObject
{
public:
	Bloom()
	{
		registerDebugRenderTarget("Bloom");
	}

	Error init();

	void importRenderTargets(RenderingContext& ctx);

	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getPyramidRt() const
	{
		return m_runCtx.m_pyramidRt;
	}

	UVec2 getPyramidTextureSize() const
	{
		return UVec2(m_pyramidTex->getWidth(), m_pyramidTex->getHeight());
	}

	U32 getPyramidTextureMipmapCount() const
	{
		return m_pyramidTex->getMipmapCount();
	}

	RenderTargetHandle getBloomRt() const
	{
		return m_runCtx.m_finalRt;
	}

	void getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
							  [[maybe_unused]] Array<DebugRenderTargetDrawStyle, kMaxDebugRenderTargets>& drawStyles) const override
	{
		handles[0] = m_runCtx.m_finalRt;
		drawStyles[0] = DebugRenderTargetDrawStyle::kTonemap;
	}

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_downscaleGrProg;
	ShaderProgramPtr m_exposureGrProg;
	ShaderProgramPtr m_upscaleGrProg;

	TexturePtr m_pyramidTex;

	RenderTargetDesc m_exposureRtDesc;
	RenderTargetDesc m_finalRtDesc;

	ImageResourcePtr m_lensDirtImg;

	class
	{
	public:
		RenderTargetHandle m_pyramidRt;
		RenderTargetHandle m_finalRt;
	} m_runCtx;
};
/// @}

} // end namespace anki
