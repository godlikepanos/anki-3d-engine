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
		return (doSharpening()) ? m_runCtx.m_sharpenedRt : m_runCtx.m_scaledRt;
	}

private:
	ShaderProgramResourcePtr m_scaleProg;
	ShaderProgramPtr m_scaleGrProg;
	ShaderProgramResourcePtr m_sharpenProg;
	ShaderProgramPtr m_sharpenGrProg;

	GrUpscalerPtr m_grUpscaler;

	FramebufferDescription m_fbDescr;
	RenderTargetDescription m_rtDesc;

	Bool m_fsr = false;

	class
	{
	public:
		RenderTargetHandle m_scaledRt;
		RenderTargetHandle m_sharpenedRt;
	} m_runCtx;

	void runScaling(RenderPassWorkContext& rgraphCtx);
	void runSharpening(RenderPassWorkContext& rgraphCtx);
	void runDLSS(RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);

	Bool doSharpening() const
	{
		return m_sharpenProg.isCreated();
	}

	Bool doDLSS() const
	{
		return m_grUpscaler.isCreated();
	}

	Bool doScaling() const
	{
		return m_scaleProg.isCreated() || doDLSS();
	}
};
/// @}

} // end namespace anki
