// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Ms.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Downsample some render targets.
class Downsample : public RenderingPass
{
anki_internal:
	Downsample(Renderer* r)
		: RenderingPass(r)
	{
	}

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

private:
	ShaderResourcePtr m_frag;
	ShaderResourcePtr m_vert;
	PipelinePtr m_ppline;
	FramebufferPtr m_fb;
	Array<TexturePtr, Ms::ATTACHMENT_COUNT> m_msRts;
	TexturePtr m_depthRt;
	TexturePtr m_isRt;
	ResourceGroupPtr m_rg;
};
/// @}

} // end namespace anki
