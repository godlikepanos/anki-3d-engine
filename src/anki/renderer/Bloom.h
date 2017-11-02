// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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

const PixelFormat BLOOM_RT_PIXEL_FORMAT(ComponentFormat::R8G8B8, TransformFormat::UNORM);

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
		return m_runCtx.m_exposureRt;
	}

private:
	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;

		F32 m_threshold = 10.0; ///< How bright it is
		F32 m_scale = 1.0;
		U32 m_width = 0;
		U32 m_height = 0;

		FramebufferDescription m_fbDescr;
		RenderTargetDescription m_rtDescr;
	} m_exposure;

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;

		U32 m_width = 0;
		U32 m_height = 0;

		RenderTargetDescription m_rtDescr;
		FramebufferDescription m_fbDescr;
	} m_upscale;

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
		TextureResourcePtr m_lensDirtTex;
	} m_sslf;

	class
	{
	public:
		RenderTargetHandle m_exposureRt;
		RenderTargetHandle m_upscaleRt;
	} m_runCtx;

	ANKI_USE_RESULT Error initExposure(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initUpscale(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initSslf(const ConfigSet& cfg);

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg)
	{
		ANKI_CHECK(initExposure(cfg));
		ANKI_CHECK(initUpscale(cfg));
		ANKI_CHECK(initSslf(cfg));
		return Error::NONE;
	}

	static void runExposureCallback(void* userData,
		CommandBufferPtr cmdb,
		U32 secondLevelCmdbIdx,
		U32 secondLevelCmdbCount,
		const RenderGraph& rgraph)
	{
		ANKI_ASSERT(userData);
		static_cast<Bloom*>(userData)->runExposure(rgraph, cmdb);
	}

	static void runUpscaleAndSslfCallback(void* userData,
		CommandBufferPtr cmdb,
		U32 secondLevelCmdbIdx,
		U32 secondLevelCmdbCount,
		const RenderGraph& rgraph)
	{
		ANKI_ASSERT(userData);
		static_cast<Bloom*>(userData)->runUpscaleAndSslf(rgraph, cmdb);
	}

	void runExposure(const RenderGraph& rgraph, CommandBufferPtr& cmdb);
	void runUpscaleAndSslf(const RenderGraph& rgraph, CommandBufferPtr& cmdb);
};

/// @}

} // end namespace anki
