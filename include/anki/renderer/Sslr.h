// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_SSLR_H
#define ANKI_RENDERER_SSLR_H

#include "anki/renderer/RenderingPass.h"
#include "anki/resource/Resource.h"
#include "anki/Gr.h"

namespace anki {

/// @addtogroup renderer
/// @{

/// Screen space local reflections pass
class Sslr: public BlurringRenderingPass
{
public:
	/// @privatesection
	/// @{
	Sslr(Renderer* r)
	:	BlurringRenderingPass(r)
	{}

	ANKI_USE_RESULT Error init(const ConfigSet& config);
	ANKI_USE_RESULT Error run(CommandBufferHandle& cmdBuff);
	/// @}

private:
	U32 m_width;
	U32 m_height;

	// 1st pass
	ShaderResourcePointer m_reflectionFrag;
	PipelineHandle m_reflectionPpline;
	SamplerHandle m_depthMapSampler;

	// 2nd pass: blit
	ShaderResourcePointer m_blitFrag;
	PipelineHandle m_blitPpline;
};

/// @}

} // end namespace anki

#endif
