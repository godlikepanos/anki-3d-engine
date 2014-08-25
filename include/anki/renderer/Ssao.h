// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_SSAO_H
#define ANKI_RENDERER_SSAO_H

#include "anki/renderer/RenderingPass.h"
#include "anki/resource/ProgramResource.h"
#include "anki/resource/TextureResource.h"
#include "anki/resource/Resource.h"
#include "anki/Gl.h"
#include "anki/core/Timestamp.h"

namespace anki {

/// @addtogroup renderer
/// @{

/// Screen space ambient occlusion pass
class Ssao: public OptionalRenderingPass
{
	friend class Pps;
	friend class Sslr;
	friend class MainRenderer;

private:
	U32 m_width, m_height; ///< Blur passes size
	U8 m_blurringIterationsCount;

	GlTextureHandle m_vblurRt;
	GlTextureHandle m_hblurRt;
	GlFramebufferHandle m_vblurFb;
	GlFramebufferHandle m_hblurFb;

	ProgramResourcePointer m_ssaoFrag;
	ProgramResourcePointer m_hblurFrag;
	ProgramResourcePointer m_vblurFrag;
	GlProgramPipelineHandle m_ssaoPpline;
	GlProgramPipelineHandle m_hblurPpline;
	GlProgramPipelineHandle m_vblurPpline;
	
	Timestamp m_commonUboUpdateTimestamp = getGlobTimestamp();
	GlBufferHandle m_uniformsBuff;
	GlTextureHandle m_noiseTex;

	Ssao(Renderer* r)
		: OptionalRenderingPass(r)
	{}

	void init(const ConfigSet& initializer);
	void run(GlCommandBufferHandle& jobs);

	GlTextureHandle& getRt()
	{
		return m_vblurRt;
	}

	void createFb(GlFramebufferHandle& fb, GlTextureHandle& rt);
	void initInternal(const ConfigSet& initializer);
};

/// @}

} // end namespace anki

#endif
