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

/// Screen space lens flare pass.
class ScreenSpaceLensFlare : public RenderingPass
{
anki_internal:
	ScreenSpaceLensFlare(Renderer* r)
		: RenderingPass(r)
	{
	}

	ANKI_USE_RESULT Error init(const ConfigSet& config);
	void run(RenderingContext& ctx);

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;
	TextureResourcePtr m_lensDirtTex;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& config);
};
/// @}

} // end namespace anki
