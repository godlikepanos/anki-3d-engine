// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Upscale some textures and append them to IS.
class FsUpscale : public RenderingPass
{
public:
	FsUpscale(Renderer* r)
		: RenderingPass(r)
	{
	}

	ANKI_USE_RESULT Error init(const ConfigSet& config);

	void run(RenderingContext& ctx);

private:
	FramebufferPtr m_fb;
	ShaderResourcePtr m_frag;
	ShaderProgramPtr m_prog;
	SamplerPtr m_nearestSampler;

	TextureResourcePtr m_noiseTex;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& config);
};
/// @}

} // end namespace anki
