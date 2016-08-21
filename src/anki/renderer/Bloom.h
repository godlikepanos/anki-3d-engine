// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>
#include <anki/Gr.h>
#include <anki/resource/TextureResource.h>
#include <anki/resource/ShaderResource.h>
#include <anki/core/Timestamp.h>

namespace anki
{

class ShaderProgram;

/// @addtogroup renderer
/// @{

/// Bloom pass.
class Bloom : public RenderingPass
{
anki_internal:
	static const PixelFormat RT_PIXEL_FORMAT;

	Bloom(Renderer* r)
		: RenderingPass(r)
	{
	}

	~Bloom();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	void setPreRunBarriers(RenderingContext& ctx);

	void run(RenderingContext& ctx);

	void setPostRunBarriers(RenderingContext& ctx);

	TexturePtr& getMaxExposureRt()
	{
		return m_extractExposure.m_rt;
	}

	TexturePtr& getFinalRt()
	{
		return m_upscale.m_rt;
	}

	U getMaxExposureRtWidth() const
	{
		return m_extractExposure.m_width;
	}

	U getMaxExposureRtHeight() const
	{
		return m_extractExposure.m_height;
	}

private:
	class SubPass
	{
	public:
		U32 m_width, m_height;

		TexturePtr m_rt;
		FramebufferPtr m_fb;

		ShaderResourcePtr m_frag;
		PipelinePtr m_ppline;
		ResourceGroupPtr m_rsrc;
	};

	F32 m_threshold = 10.0; ///< How bright it is
	F32 m_scale = 1.0;

	SubPass m_extractExposure;
	SubPass m_upscale;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);
};

/// @}

} // end namespace anki
