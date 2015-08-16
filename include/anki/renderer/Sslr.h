// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/renderer/RenderingPass.h"
#include "anki/Gr.h"

namespace anki {

/// @addtogroup renderer
/// @{

/// Screen space local reflections pass
class Sslr: public RenderingPass
{
anki_internal:
	Sslr(Renderer* r)
		: RenderingPass(r)
	{}

	ANKI_USE_RESULT Error init(const ConfigSet& config);
	void run(CommandBufferPtr& cmdBuff);

private:
	U32 m_width;
	U32 m_height;

	// 1st pass
	ShaderResourcePtr m_reflectionFrag;
	PipelinePtr m_reflectionPpline;
	SamplerPtr m_depthMapSampler;
	TexturePtr m_rt;
	FramebufferPtr m_fb;
	ResourceGroupPtr m_rcGroup;
	ResourceGroupPtr m_rcGroupBlit;

	// 2nd pass: blit
	FramebufferPtr m_isFb;
	ShaderResourcePtr m_blitFrag;
	PipelinePtr m_blitPpline;
};
/// @}

} // end namespace anki

