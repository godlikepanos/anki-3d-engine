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

/// Tonemapping.
class Tm : public RenderingPass
{
anki_internal:
	Tm(Renderer* r)
		: RenderingPass(r)
	{
	}

	BufferPtr& getAverageLuminanceBuffer()
	{
		return m_luminanceBuff;
	}

	ANKI_USE_RESULT Error create(const ConfigSet& initializer);

	void run(RenderingContext& ctx);

private:
	ShaderResourcePtr m_luminanceShader;
	PipelinePtr m_luminancePpline;
	BufferPtr m_luminanceBuff;
	ResourceGroupPtr m_rcGroup;
};
/// @}

} // end namespace anki
