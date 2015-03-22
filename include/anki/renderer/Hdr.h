// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_HDR_H
#define ANKI_RENDERER_HDR_H

#include "anki/renderer/RenderingPass.h"
#include "anki/Gr.h"
#include "anki/resource/TextureResource.h"
#include "anki/resource/ShaderResource.h"
#include "anki/resource/Resource.h"
#include "anki/core/Timestamp.h"

namespace anki {

class ShaderProgram;

/// @addtogroup renderer
/// @{

/// High dynamic range lighting pass
class Hdr: public RenderingPass
{
	friend class Pps;

public:
	/// @name Accessors
	/// @{
	F32 getExposure() const
	{
		return m_exposure;
	}
	void setExposure(const F32 x)
	{
		m_exposure = x;
		m_parameterUpdateTimestamp = getGlobalTimestamp();
	}

	U32 getBlurringIterationsCount() const
	{
		return m_blurringIterationsCount;
	}
	void setBlurringIterationsCount(const U32 x)
	{
		m_blurringIterationsCount = x;
	}
	/// @}

	/// @privatesection
	/// @{
	TextureHandle& _getRt()
	{
		return m_vblurRt;
	}

	U32 _getWidth() const
	{
		return m_width;
	}

	U32 _getHeight() const
	{
		return m_height;
	}
	/// @}

private:
	U32 m_width, m_height;
	F32 m_exposure = 4.0; ///< How bright is the HDR
	U32 m_blurringIterationsCount = 2; ///< The blurring iterations
	F32 m_blurringDist = 1.0; ///< Distance in blurring
	
	FramebufferHandle m_hblurFb;
	FramebufferHandle m_vblurFb;

	ShaderResourcePointer m_toneFrag;
	ShaderResourcePointer m_hblurFrag;
	ShaderResourcePointer m_vblurFrag;

	PipelineHandle m_tonePpline;
	PipelineHandle m_hblurPpline;
	PipelineHandle m_vblurPpline;

	TextureHandle m_hblurRt; ///< pass0Fai with the horizontal blur FAI
	TextureHandle m_vblurRt; ///< The final FAI
	/// When a parameter changed by the setters
	Timestamp m_parameterUpdateTimestamp = 0;
	/// When the commonUbo got updated
	Timestamp m_commonUboUpdateTimestamp = 0;
	BufferHandle m_commonBuff;

	Hdr(Renderer* r)
	:	RenderingPass(r)
	{}

	~Hdr();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);
	ANKI_USE_RESULT Error run(CommandBufferHandle& jobs);

	ANKI_USE_RESULT Error initFb(FramebufferHandle& fb, TextureHandle& rt);
	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);

	ANKI_USE_RESULT Error updateDefaultBlock(CommandBufferHandle& jobs);
};

/// @}

} // end namespace anki

#endif
