// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_LF_H
#define ANKI_RENDERER_LF_H

#include "anki/renderer/RenderingPass.h"
#include "anki/Gl.h"
#include "anki/resource/Resource.h"
#include "anki/resource/ProgramResource.h"
#include "anki/resource/TextureResource.h"

namespace anki {

/// @addtogroup renderer
/// @{

/// Lens flare rendering pass
class Lf: public RenderingPass
{
public:
	/// @privatesection
	/// @{
	Lf(Renderer* r)
	:	RenderingPass(r)
	{}

	~Lf();

	const GlTextureHandle& _getRt() const
	{
		return m_rt;
	}

	GlTextureHandle& _getRt()
	{
		return m_rt;
	}

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	ANKI_USE_RESULT Error runOcclusionTests(GlCommandBufferHandle& cmdb);

	ANKI_USE_RESULT Error run(GlCommandBufferHandle& jobs);
	/// @}

private:
	GlTextureHandle m_rt;
	GlFramebufferHandle m_fb;

	// Occlusion query
	GlBufferHandle m_positionsVertBuff;
	GlBufferHandle m_mvpBuff;
	ProgramResourcePointer m_occlusionVert;
	ProgramResourcePointer m_occlusionFrag;
	GlPipelineHandle m_occlusionPpline;

	// Pseudo flares
	ProgramResourcePointer m_pseudoFrag;
	GlPipelineHandle m_pseudoPpline;
	TextureResourcePointer m_lensDirtTex;
	U8 m_maxSpritesPerFlare;
	U8 m_maxFlares;

	// Sprite billboards
	ProgramResourcePointer m_realVert;
	ProgramResourcePointer m_realFrag;
	GlPipelineHandle m_realPpline;
	GlBufferHandle m_flareDataBuff;
	U32 m_flareSize;

	// Final HDR blit
	ProgramResourcePointer m_blitFrag;
	GlPipelineHandle m_blitPpline;

	ANKI_USE_RESULT Error initPseudo(
		const ConfigSet& config, GlCommandBufferHandle& cmdBuff);
	ANKI_USE_RESULT Error initSprite(
		const ConfigSet& config, GlCommandBufferHandle& cmdBuff);
	ANKI_USE_RESULT Error initOcclusion(
		const ConfigSet& config, GlCommandBufferHandle& cmdBuff);

	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);
};

/// @}

} // end namespace anki

#endif
