// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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
public:
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

	F32 getThreshold() const
	{
		return m_exposure.m_threshold;
	}

	void setThreshold(F32 t)
	{
		m_exposure.m_threshold = t;
	}

	F32 getScale() const
	{
		return m_exposure.m_scale;
	}

	void setScale(F32 t)
	{
		m_exposure.m_scale = t;
	}

private:
	static constexpr Format RT_PIXEL_FORMAT = Format::A2B10G10R10_UNORM_PACK32;

	const Array<U32, 3> m_workgroupSize = {16, 16, 1};

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;

		F32 m_threshold = 10.0f; ///< How bright it is
		F32 m_scale = 1.0f;
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

	void runExposure(RenderPassWorkContext& rgraphCtx);
	void runUpscaleAndSslf(RenderPassWorkContext& rgraphCtx);
};

/// @}

} // end namespace anki
