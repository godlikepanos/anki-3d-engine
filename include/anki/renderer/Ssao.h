// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>
#include <anki/resource/ShaderResource.h>
#include <anki/resource/TextureResource.h>
#include <anki/Gr.h>
#include <anki/core/Timestamp.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Screen space ambient occlusion pass
class Ssao : public RenderingPass
{
anki_internal:
	static const PixelFormat RT_PIXEL_FORMAT;

	Ssao(Renderer* r)
		: RenderingPass(r)
	{
	}

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);
	void run(RenderingContext& ctx);

	TexturePtr& getRt()
	{
		return m_vblurRt;
	}

private:
	U32 m_width, m_height; ///< Blur passes size
	U8 m_blurringIterationsCount;

	TexturePtr m_vblurRt;
	TexturePtr m_hblurRt;
	FramebufferPtr m_vblurFb;
	FramebufferPtr m_hblurFb;

	ShaderResourcePtr m_quadVert;
	ShaderResourcePtr m_ssaoFrag;
	ShaderResourcePtr m_hblurFrag;
	ShaderResourcePtr m_vblurFrag;
	PipelinePtr m_ssaoPpline;
	PipelinePtr m_hblurPpline;
	PipelinePtr m_vblurPpline;

	TexturePtr m_noiseTex;

	ResourceGroupPtr m_rcFirst;
	ResourceGroupPtr m_hblurRc;
	ResourceGroupPtr m_vblurRc;

	ANKI_USE_RESULT Error createFb(FramebufferPtr& fb, TexturePtr& rt);
	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);
};
/// @}

} // end namespace anki
