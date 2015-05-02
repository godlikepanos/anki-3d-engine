// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_SSAO_H
#define ANKI_RENDERER_SSAO_H

#include "anki/renderer/RenderingPass.h"
#include "anki/resource/ShaderResource.h"
#include "anki/resource/TextureResource.h"
#include "anki/resource/Resource.h"
#include "anki/Gr.h"
#include "anki/core/Timestamp.h"

namespace anki {

/// @addtogroup renderer
/// @{

/// Screen space ambient occlusion pass
class Ssao: public RenderingPass
{
public:
	/// @privatesection
	/// @{
	Ssao(Renderer* r)
	:	RenderingPass(r)
	{}

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);
	ANKI_USE_RESULT Error run(CommandBufferHandle& cmdBuff);

	TextureHandle& _getRt()
	{
		return m_vblurRt;
	}

	BufferHandle& getUniformBuffer()
	{
		return m_uniformsBuff;
	}
	/// @}

private:
	U32 m_width, m_height; ///< Blur passes size
	U8 m_blurringIterationsCount;

	TextureHandle m_vblurRt;
	TextureHandle m_hblurRt;
	FramebufferHandle m_vblurFb;
	FramebufferHandle m_hblurFb;

	ShaderResourcePointer m_ssaoFrag;
	ShaderResourcePointer m_hblurFrag;
	ShaderResourcePointer m_vblurFrag;
	PipelineHandle m_ssaoPpline;
	PipelineHandle m_hblurPpline;
	PipelineHandle m_vblurPpline;
	
	Timestamp m_commonUboUpdateTimestamp = 0;
	BufferHandle m_uniformsBuff;
	TextureHandle m_noiseTex;

	ANKI_USE_RESULT Error createFb(
		FramebufferHandle& fb, TextureHandle& rt);
	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);
};

/// @}

} // end namespace anki

#endif
