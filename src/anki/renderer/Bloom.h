// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>
#include <anki/Gr.h>
#include <anki/resource/TextureResource.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Bloom passes.
class Bloom : public RendererObject
{
anki_internal:
	Bloom(Renderer* r);

	~Bloom();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg)
	{
		ANKI_R_LOGI("Initializing bloom passes");
		Error err = initInternal(cfg);
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
	static const Format RT_PIXEL_FORMAT = Format::A2B10G10R10_UNORM_PACK32;

	Array<U32, 2> m_workgroupSize = {{16, 16}};

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;

		F32 m_threshold = 10.0; ///< How bright it is
		F32 m_scale = 1.0;
		U32 m_width = 0;
		U32 m_height = 0;

		RenderTargetDescription m_rtDescr;
	} m_exposure;

	class
	{
	public:
		TextureResourcePtr m_lensDirtTex;
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

	ANKI_USE_RESULT Error initExposure(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initUpscale(const ConfigSet& cfg);

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg)
	{
		ANKI_CHECK(initExposure(cfg));
		ANKI_CHECK(initUpscale(cfg));
		return Error::NONE;
	}

	static void runExposureCallback(RenderPassWorkContext& rgraphCtx)
	{
		scast<Bloom*>(rgraphCtx.m_userData)->runExposure(rgraphCtx);
	}

	static void runUpscaleAndSslfCallback(RenderPassWorkContext& rgraphCtx)
	{
		scast<Bloom*>(rgraphCtx.m_userData)->runUpscaleAndSslf(rgraphCtx);
	}

	void runExposure(RenderPassWorkContext& rgraphCtx);
	void runUpscaleAndSslf(RenderPassWorkContext& rgraphCtx);
};

/// @}

} // end namespace anki
