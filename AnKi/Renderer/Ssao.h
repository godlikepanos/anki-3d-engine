// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Gr.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Resolves shadowmaps into a single texture.
class Ssao : public RendererObject
{
public:
	Ssao()
	{
		registerDebugRenderTarget("Ssao");
	}

	Error init();

	void populateRenderGraph(RenderingContext& ctx);

	void getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
							  [[maybe_unused]] ShaderProgramPtr& optionalShaderProgram) const override
	{
		ANKI_ASSERT(rtName == "Ssao");
		handles[0] = m_runCtx.m_ssaoRts[1];
	}

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_ssaoRts[1];
	}

public:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;
	Array<ShaderProgramPtr, 2> m_denoiseGrProgs;

	FramebufferDescription m_fbDescr;
	ImageResourcePtr m_noiseImage;

	Array<TexturePtr, 2> m_rts;
	Bool m_rtsImportedOnce = false;

	class
	{
	public:
		Array<RenderTargetHandle, 2> m_ssaoRts;
	} m_runCtx;

	Error initInternal();
};
/// @}

} // end namespace anki
