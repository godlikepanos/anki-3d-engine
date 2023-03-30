// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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
	Error init();

	void importRenderTargets(RenderingContext& ctx);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	/// @copydoc m_exposureAndAvgLuminance1x1
	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_exposureLuminanceHandle;
	}

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;
	U32 m_inputTexMip;

	/// This is a 1x1 2 component texture where R is the exposure and G the average luminance. It's not tracked in
	/// rendergraph depedencies. We don't care to track it because it affects the eye adaptation.
	TexturePtr m_exposureAndAvgLuminance1x1;

	class
	{
	public:
		RenderTargetHandle m_exposureLuminanceHandle;
	} m_runCtx;

	Error initInternal();
};
/// @}

} // end namespace anki
