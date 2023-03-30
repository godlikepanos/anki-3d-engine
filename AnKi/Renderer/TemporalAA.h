// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Temporal AA resolve.
class TemporalAA : public RendererObject
{
public:
	Error init();

	void populateRenderGraph(RenderingContext& ctx);

	/// Result is tonemaped.
	RenderTargetHandle getTonemappedRt() const
	{
		return m_runCtx.m_tonemappedRt;
	}

	RenderTargetHandle getHdrRt() const
	{
		return m_runCtx.m_renderRt;
	}

private:
	Array<TexturePtr, 2> m_rtTextures;
	Array<Bool, 2> m_rtTexturesImportedOnce = {};

	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	RenderTargetDescription m_tonemappedRtDescr;
	FramebufferDescription m_fbDescr;

	class
	{
	public:
		RenderTargetHandle m_renderRt;
		RenderTargetHandle m_historyRt;
		RenderTargetHandle m_tonemappedRt;
	} m_runCtx;

	Error initInternal();
};
/// @}

} // end namespace anki
