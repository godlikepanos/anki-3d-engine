// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki
{

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

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_upscaledRt;
	}

private:
	ShaderProgramResourcePtr m_blitProg;
	ShaderProgramPtr m_blitGrProg;

	RenderTargetDescription m_rtDesc;
	FramebufferDescription m_fbDescr;

	class
	{
	public:
		RenderTargetHandle m_upscaledRt;
	} m_runCtx;

	void run(RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
