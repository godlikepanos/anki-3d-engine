// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{
ANKI_CVAR(BoolCVar, Render, Dlss, true, "Enable or disable DLSS")
ANKI_CVAR(NumericCVar<U8>, Render, DlssQuality, 1, 0, 3, "0: Performance, 1: Balanced, 2: Quality")

/// Upscales.
class TemporalUpscaler : public RendererObject
{
public:
	Error init();

	void populateRenderGraph(RenderingContext& ctx);

	/// This is the HDR upscaled RT.
	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

	Bool getEnabled() const
	{
		return m_grUpscaler.isCreated();
	}

private:
	GrUpscalerPtr m_grUpscaler;

	RenderTargetDesc m_rtDesc;

	ShaderProgramResourcePtr m_blitProg;
	ShaderProgramPtr m_blitGrProg;

	class
	{
	public:
		RenderTargetHandle m_rt;
	} m_runCtx;
};
/// @}

} // end namespace anki
