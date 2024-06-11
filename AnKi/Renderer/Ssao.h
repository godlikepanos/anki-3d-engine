// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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

/// Screen space ambient occlusion.
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
		handles[0] = m_runCtx.m_finalRt;
	}

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_finalRt;
	}

public:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;
	ShaderProgramPtr m_spatialDenoiseGrProg;
	ShaderProgramPtr m_tempralDenoiseGrProg;

	RenderTargetDesc m_bentNormalsAndSsaoRtDescr;

	Array<TexturePtr, 2> m_tex;
	Bool m_texImportedOnce = false;

	ImageResourcePtr m_noiseImage;

	class
	{
	public:
		RenderTargetHandle m_finalRt;
	} m_runCtx;

	Error initInternal();
};
/// @}

} // end namespace anki
