// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>
#include <anki/Gr.h>
#include <anki/resource/TextureResource.h>
#include <anki/resource/ShaderResource.h>
#include <anki/core/Timestamp.h>

namespace anki {

class ShaderProgram;

/// @addtogroup renderer
/// @{

/// Bloom pass.
class Bloom: public RenderingPass
{
public:
	U32 getBlurringIterationsCount() const
	{
		return m_blurringIterationsCount;
	}

	void setBlurringIterationsCount(const U32 x)
	{
		m_blurringIterationsCount = x;
	}

anki_internal:
	static const PixelFormat RT_PIXEL_FORMAT;

	Bloom(Renderer* r)
		: RenderingPass(r)
	{}

	~Bloom();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);
	void run(CommandBufferPtr& jobs);

	TexturePtr& getRt()
	{
		return m_vblurRt;
	}

	U32 getWidth() const
	{
		return m_width;
	}

	U32 getHeight() const
	{
		return m_height;
	}

private:
	U32 m_width, m_height;
	F32 m_threshold = 10.0; ///< How bright it is
	F32 m_scale = 1.0;
	U32 m_blurringIterationsCount = 2; ///< The blurring iterations
	F32 m_blurringDist = 1.0; ///< Distance in blurring

	FramebufferPtr m_hblurFb;
	FramebufferPtr m_vblurFb;

	ShaderResourcePtr m_toneFrag;
	ShaderResourcePtr m_hblurFrag;
	ShaderResourcePtr m_vblurFrag;

	PipelinePtr m_tonePpline;
	PipelinePtr m_hblurPpline;
	PipelinePtr m_vblurPpline;

	TexturePtr m_hblurRt; ///< pass0Fai with the horizontal blur FAI
	TexturePtr m_vblurRt; ///< The final FAI
	/// When a parameter changed by the setters
	Timestamp m_parameterUpdateTimestamp = 0;
	/// When the commonUbo got updated
	Timestamp m_commonUboUpdateTimestamp = 0;
	BufferPtr m_commonBuff;

	ResourceGroupPtr m_firstDescrGroup;
	ResourceGroupPtr m_hDescrGroup;
	ResourceGroupPtr m_vDescrGroup;

	ANKI_USE_RESULT Error initFb(FramebufferPtr& fb, TexturePtr& rt);
	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);

	void updateDefaultBlock(CommandBufferPtr& jobs);
};

/// @}

} // end namespace anki

