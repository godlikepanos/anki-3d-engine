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

/// Tonemapping.
class Tonemapping : public RenderingPass
{
anki_internal:
	BufferPtr m_luminanceBuff;

	Tonemapping(Renderer* r)
		: RenderingPass(r)
	{
	}

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	void run(RenderingContext& ctx);

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;
	U8 m_rtIdx;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);
};
/// @}

} // end namespace anki
