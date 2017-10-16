// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>
#include <anki/renderer/ScreenSpaceLensFlare.h>
#include <anki/Gr.h>
#include <anki/resource/TextureResource.h>

namespace anki
{

/// @addtogroup renderer
/// @{

const PixelFormat BLOOM_RT_PIXEL_FORMAT(ComponentFormat::R8G8B8, TransformFormat::UNORM);

class BloomExposure : public RendererObject
{
anki_internal:
	U32 m_width = 0;
	U32 m_height = 0;
	TexturePtr m_rt;

	BloomExposure(Renderer* r)
		: RendererObject(r)
	{
	}

	~BloomExposure();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	void setPreRunBarriers(RenderingContext& ctx);

	void run(RenderingContext& ctx);

	void setPostRunBarriers(RenderingContext& ctx);

private:
	FramebufferPtr m_fb;

	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	F32 m_threshold = 10.0; ///< How bright it is
	F32 m_scale = 1.0;
};

class BloomUpscale : public RendererObject
{
anki_internal:
	U32 m_width = 0;
	U32 m_height = 0;
	TexturePtr m_rt;

	BloomUpscale(Renderer* r)
		: RendererObject(r)
	{
	}

	~BloomUpscale();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	void setPreRunBarriers(RenderingContext& ctx);

	void run(RenderingContext& ctx);

	void setPostRunBarriers(RenderingContext& ctx);

private:
	FramebufferPtr m_fb;

	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;
};

/// Bloom passes.
class Bloom : public RendererObject
{
anki_internal:
	BloomExposure m_extractExposure;
	BloomUpscale m_upscale;
	ScreenSpaceLensFlare m_sslf;

	Bloom(Renderer* r)
		: RendererObject(r)
		, m_extractExposure(r)
		, m_upscale(r)
		, m_sslf(r)
	{
	}

	~Bloom()
	{
	}

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

private:
	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg)
	{
		ANKI_CHECK(m_extractExposure.init(cfg));
		ANKI_CHECK(m_upscale.init(cfg));
		ANKI_CHECK(m_sslf.init(cfg));
		return Error::NONE;
	}
};

/// @}

} // end namespace anki
