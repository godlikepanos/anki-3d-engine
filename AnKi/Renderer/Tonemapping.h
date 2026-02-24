// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

// Tonemapping.
class Tonemapping : public RendererObject
{
public:
	Error init();

	void importRenderTargets();

	void populateRenderGraph();

	// See m_exposureAndAvgLuminance1x1
	RenderTargetHandle getExposureAndAvgLuminanceRt() const
	{
		return m_runCtx.m_exposureLuminanceHandle;
	}

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

private:
	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
		U32 m_inputTexMip;

		// This is a 1x1 2 component texture where R is the exposure and G the average luminance. It's not tracked in
		// rendergraph depedencies. We don't care to track it because it affects the eye adaptation.
		RendererTexture m_exposureAndAvgLuminance1x1;
	} m_expAndAvgLum;

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;

		RenderTargetDesc m_rtDesc;

		ImageResourcePtr m_lut; ///< Color grading lookup texture.
	} m_tonemapping;

	class
	{
	public:
		RenderTargetHandle m_exposureLuminanceHandle;
		RenderTargetHandle m_rt;
	} m_runCtx;
};

} // end namespace anki
