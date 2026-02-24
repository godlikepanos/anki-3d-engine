// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

// Temporal AA resolve.
class TemporalAA : public RendererObject
{
public:
	Error init();

	void populateRenderGraph();

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_renderRt;
	}

private:
	Array<RendererTexture, 2> m_rtTextures;
	Bool m_rtTexturesImportedOnce = false;

	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	class
	{
	public:
		RenderTargetHandle m_renderRt;
		RenderTargetHandle m_historyRt;
	} m_runCtx;
};

} // end namespace anki
