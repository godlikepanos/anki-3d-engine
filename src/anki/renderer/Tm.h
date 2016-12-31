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
class Tm : public RenderingPass
{
anki_internal:
	BufferPtr m_luminanceBuff;

	Tm(Renderer* r)
		: RenderingPass(r)
	{
	}

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	void run(RenderingContext& ctx);

private:
	ShaderResourcePtr m_luminanceShader;
	ShaderProgramPtr m_prog;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);
};
/// @}

} // end namespace anki
