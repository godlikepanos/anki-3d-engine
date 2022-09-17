// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Gr.h>
#include <AnKi/Resource/ImageResource.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Bloom passes.
class Bloom : public RendererObject
{
public:
	Bloom(Renderer* r);

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
	static constexpr Format kRtPixelFormat = Format::kA2B10G10R10_Unorm_Pack32;

	const Array<U32, 2> m_workgroupSize = {16, 16};

	FramebufferDescription m_fbDescr;

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;

		U32 m_width = 0;
		U32 m_height = 0;

		RenderTargetDescription m_rtDescr;
	} m_exposure;

	class
	{
	public:
		ImageResourcePtr m_lensDirtImage;
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;

		U32 m_width = 0;
		U32 m_height = 0;

		RenderTargetDescription m_rtDescr;
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
