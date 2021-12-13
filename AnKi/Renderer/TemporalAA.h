// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
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
	TemporalAA(Renderer* r);

	~TemporalAA();

	ANKI_USE_RESULT Error init();

	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getHdrRt() const
	{
		return m_runCtx.m_renderRt;
	}

	RenderTargetHandle getTonemappedRt() const
	{
		return m_runCtx.m_tonemappedRt;
	}

private:
	Array<TexturePtr, 2> m_rtTextures;
	Array<Bool, 2> m_rtTexturesImportedOnce = {};

	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	RenderTargetDescription m_tonemappedRtDescr;

	class
	{
	public:
		RenderTargetHandle m_renderRt;
		RenderTargetHandle m_historyRt;
		RenderTargetHandle m_tonemappedRt;
	} m_runCtx;

	ANKI_USE_RESULT Error initInternal();
};
/// @}

} // end namespace anki
