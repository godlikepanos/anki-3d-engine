// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>
#include <anki/renderer/Sslf.h>
#include <anki/Gr.h>
#include <anki/resource/TextureResource.h>
#include <anki/resource/ShaderResource.h>

namespace anki
{

/// @addtogroup renderer
/// @{

const PixelFormat BLOOM_RT_PIXEL_FORMAT(ComponentFormat::R8G8B8, TransformFormat::UNORM);

class BloomExposure : public RenderingPass
{
anki_internal:
	U32 m_width = 0;
	U32 m_height = 0;
	TexturePtr m_rt;

	BloomExposure(Renderer* r)
		: RenderingPass(r)
	{
	}

	~BloomExposure();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	void setPreRunBarriers(RenderingContext& ctx);

	void run(RenderingContext& ctx);

	void setPostRunBarriers(RenderingContext& ctx);

private:
	FramebufferPtr m_fb;
	ShaderResourcePtr m_frag;
	ShaderProgramPtr m_prog;

	F32 m_threshold = 10.0; ///< How bright it is
	F32 m_scale = 1.0;
};

class BloomUpscale : public RenderingPass
{
anki_internal:
	U32 m_width = 0;
	U32 m_height = 0;
	TexturePtr m_rt;

	BloomUpscale(Renderer* r)
		: RenderingPass(r)
	{
	}

	~BloomUpscale();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	void setPreRunBarriers(RenderingContext& ctx);

	void run(RenderingContext& ctx);

	void setPostRunBarriers(RenderingContext& ctx);

private:
	FramebufferPtr m_fb;
	ShaderResourcePtr m_frag;
	ShaderProgramPtr m_prog;
};

/// Bloom pass.
class Bloom : public RenderingPass
{
anki_internal:
	BloomExposure m_extractExposure;
	BloomUpscale m_upscale;
	Sslf m_sslf;

	Bloom(Renderer* r)
		: RenderingPass(r)
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
		return ErrorCode::NONE;
	}
};

/// @}

} // end namespace anki
