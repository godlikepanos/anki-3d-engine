// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// XXX
class Velocity : public RenderingPass
{
anki_internal:
	TexturePtr m_rt;

	Velocity(Renderer* r)
		: RenderingPass(r)
	{
	}

	~Velocity();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	void setPreRunBarriers(RenderingContext& ctx)
	{
		// XXX
	}

	void run(RenderingContext& ctx);

	void setPostRunBarriers(RenderingContext& ctx)
	{
		// XXX
	}

private:
	ShaderResourcePtr m_frag;
	ShaderProgramPtr m_prog;
	FramebufferPtr m_fb;

	Error initInternal(const ConfigSet& cfg);
};
/// @}

} // end namespace anki

