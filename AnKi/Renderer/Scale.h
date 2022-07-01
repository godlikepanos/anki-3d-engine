// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Upscale or downscale pass.
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

	RenderTargetHandle getRt() const
	{
		return (m_sharpenMethod != SharpenMethod::NONE) ? m_runCtx.m_sharpenedRt : m_runCtx.m_scaledRt;
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

	GrUpscalerPtr m_grUpscaler;

	FramebufferDescription m_fbDescr;
	RenderTargetDescription m_rtDesc;

	enum class UpscalingMethod : U8
	{
		NONE,
		BILINEAR,
		FSR,
		GR
	};

	UpscalingMethod m_upscalingMethod = UpscalingMethod::NONE;

	enum class SharpenMethod : U8
	{
		NONE,
		RCAS,
		GR
	};

	SharpenMethod m_sharpenMethod = SharpenMethod::NONE;

	class
	{
	public:
		RenderTargetHandle m_scaledRt;
		RenderTargetHandle m_sharpenedRt;
	} m_runCtx;

	void runFsrOrBilinearScaling(RenderPassWorkContext& rgraphCtx);
	void runRcasSharpening(RenderPassWorkContext& rgraphCtx);
	void runGrUpscaling(RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
