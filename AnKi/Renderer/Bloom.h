// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
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

	ANKI_USE_RESULT Error init()
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
	static constexpr Format RT_PIXEL_FORMAT = Format::A2B10G10R10_UNORM_PACK32;

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

	ANKI_USE_RESULT Error initExposure();
	ANKI_USE_RESULT Error initUpscale();

	ANKI_USE_RESULT Error initInternal();
};

/// @}

} // end namespace anki
