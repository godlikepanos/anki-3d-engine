// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

inline NumericCVar<F32> g_bloomThresholdCVar("R", "BloomThreshold", 2.5f, 0.0f, 256.0f, "Bloom threshold");
inline NumericCVar<F32> g_bloomScaleCVar("R", "BloomScale", 2.5f, 0.0f, 256.0f, "Bloom scale");
inline NumericCVar<U32> g_bloomPyramidLowLimit("R", "BloomPyramidLowLimit", 32, 8, 1024, "Downscale the boom pyramid up to that size");
inline NumericCVar<U32> g_bloomUpscaleDivisor("R", "BloomUpscaleDivisor", 4, 1, 1024, "Defines the resolution of the final bloom result");

/// Contains multiple post-process passes that operate on the HDR output.
class Bloom : public RendererObject
{
public:
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
