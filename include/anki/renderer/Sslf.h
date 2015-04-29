// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_SSLF_H
#define ANKI_RENDERER_SSLF_H

#include "anki/renderer/RenderingPass.h"

namespace anki {

/// @addtogroup renderer
/// @{

/// Screen space lens flare pass.
class Sslf: public RenderingPass
{
public:
	Sslf(Renderer* r)
	:	RenderingPass(r)
	{}

	~Sslf();

	ANKI_USE_RESULT Error init(const ConfigSet& config);
	void run(CommandBufferHandle& cmdb);

	TextureHandle& getRt()
	{
		return m_rt;
	}

private:
	TextureHandle m_rt;
	FramebufferHandle m_fb;
	ShaderResourcePointer m_frag;
	PipelineHandle m_ppline;
	TextureResourcePointer m_lensDirtTex;
	U8 m_maxSpritesPerFlare;
	U8 m_maxFlares;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& config);
};
/// @}

} // end namespace anki

#endif

