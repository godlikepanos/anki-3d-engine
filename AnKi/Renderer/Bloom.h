// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Gr.h>
#include <AnKi/Resource/ImageResource.h>

namespace anki {

// Forward
extern NumericCVar<F32> g_bloomThresholdCVar;

/// @addtogroup renderer
/// @{

/// Bloom passes.
class Bloom : public RendererObject
{
public:
	Bloom();

	~Bloom();

	Error init()
	{
		const Error err = initInternal();
		if(err)
		{
			ANKI_R_LOGE("Failed to initialize bloom passes");
		}
		return err;
	}

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_upscaleRt;
	}

private:
	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;

		RenderTargetDesc m_rtDescr;
	} m_exposure;

	class
	{
	public:
		ImageResourcePtr m_lensDirtImage;
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;

		RenderTargetDesc m_rtDescr;
	} m_upscale;

	class
	{
	public:
		RenderTargetHandle m_exposureRt;
		RenderTargetHandle m_upscaleRt;
	} m_runCtx;

	Error initExposure();
	Error initUpscale();

	Error initInternal();

	void getDebugRenderTarget(CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
							  [[maybe_unused]] ShaderProgramPtr& optionalShaderProgram) const override;
};

/// @}

} // end namespace anki
