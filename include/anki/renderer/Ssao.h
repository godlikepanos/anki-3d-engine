// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_SSAO_H
#define ANKI_RENDERER_SSAO_H

#include "anki/renderer/RenderingPass.h"
#include "anki/resource/ShaderResource.h"
#include "anki/resource/TextureResource.h"
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
		: RenderingPass(r)
	{}

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);
	void run(CommandBufferPtr& cmdBuff);

	TexturePtr& getRt()
	{
		return m_vblurRt;
	}

	BufferPtr& getUniformBuffer()
	{
		return m_uniformsBuff;
	}
	/// @}

private:
	U32 m_width, m_height; ///< Blur passes size
	U8 m_blurringIterationsCount;

	TexturePtr m_vblurRt;
	TexturePtr m_hblurRt;
	FramebufferPtr m_vblurFb;
	FramebufferPtr m_hblurFb;

	ShaderResourcePointer m_ssaoFrag;
	ShaderResourcePointer m_hblurFrag;
	ShaderResourcePointer m_vblurFrag;
	PipelinePtr m_ssaoPpline;
	PipelinePtr m_hblurPpline;
	PipelinePtr m_vblurPpline;

	Timestamp m_commonUboUpdateTimestamp = 0;
	BufferPtr m_uniformsBuff;
	TexturePtr m_noiseTex;

	ANKI_USE_RESULT Error createFb(
		FramebufferPtr& fb, TexturePtr& rt);
	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);
};
/// @}

} // end namespace anki

#endif
