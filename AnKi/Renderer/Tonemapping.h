// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Tonemapping.
class Tonemapping : public RendererObject
{
public:
	Tonemapping(Renderer* r)
		: RendererObject(r)
	{
	}

	Error init();

	void importRenderTargets(RenderingContext& ctx);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getExposureLuminanceRT() const
	{
		return m_runCtx.m_exposureLuminanceHandle;
	}

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;
	U32 m_inputTexMip;

	TexturePtr m_exposureLuminance1x1;

	class
	{
	public:
		RenderTargetHandle m_exposureLuminanceHandle;
	} m_runCtx;

	Error initInternal();
};
/// @}

} // end namespace anki
