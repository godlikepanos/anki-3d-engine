// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Upscales, sharpens and in some cases tonemaps.
class Scale : public RendererObject
{
public:
	Scale(Renderer* r)
		: RendererObject(r)
	{
	}

	~Scale();

	Error init();

	void populateRenderGraph(RenderingContext& ctx);

	/// This is the tonemapped, upscaled and sharpened RT.
	RenderTargetHandle getTonemappedRt() const
	{
		return m_runCtx.m_sharpenedRt;
	}

	/// This is the HDR upscaled RT. It's available if hasUscaledHdrRt() returns true.
	RenderTargetHandle getUpscaledHdrRt() const
	{
		ANKI_ASSERT(hasUpscaledHdrRt());
		return m_runCtx.m_upscaledHdrRt;
	}

	/// @see getUpscaledHdrRt.
	Bool hasUpscaledHdrRt() const
	{
		return m_runCtx.m_upscaledHdrRt.isValid();
	}

	Bool getUsingGrUpscaler() const
	{
		return m_grUpscaler.isCreated();
	}

private:
	ShaderProgramResourcePtr m_scaleProg;
	ShaderProgramPtr m_scaleGrProg;
	ShaderProgramResourcePtr m_sharpenProg;
	ShaderProgramPtr m_sharpenGrProg;
	ShaderProgramResourcePtr m_tonemapProg;
	ShaderProgramPtr m_tonemapGrProg;

	GrUpscalerPtr m_grUpscaler;

	FramebufferDescription m_fbDescr;
	RenderTargetDescription m_upscaleAndSharpenRtDescr;
	RenderTargetDescription m_tonemapedRtDescr;

	enum class UpscalingMethod : U8
	{
		kNone,
		kBilinear,
		kFsr,
		kGr,
		kCount
	};

	UpscalingMethod m_upscalingMethod = UpscalingMethod::kNone;

	enum class SharpenMethod : U8
	{
		kNone,
		kRcas,
		kCount
	};

	SharpenMethod m_sharpenMethod = SharpenMethod::kNone;

	Bool m_neeedsTonemapping = false;

	class
	{
	public:
		RenderTargetHandle m_upscaledTonemappedRt;
		RenderTargetHandle m_upscaledHdrRt;
		RenderTargetHandle m_tonemappedRt;
		RenderTargetHandle m_sharpenedRt; ///< It's tonemaped.
	} m_runCtx;

	void runFsrOrBilinearScaling(RenderPassWorkContext& rgraphCtx);
	void runRcasSharpening(RenderPassWorkContext& rgraphCtx);
	void runGrUpscaling(RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);
	void runTonemapping(RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
