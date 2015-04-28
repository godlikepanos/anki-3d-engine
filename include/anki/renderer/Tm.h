// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_TM_H
#define ANKI_RENDERER_TM_H

#include "anki/renderer/RenderingPass.h"

namespace anki {

/// @addtogroup renderer
/// @{

/// Tonemapping.
class Tm: private RenderingPass
{
public:
	Tm(Renderer* r)
	:	RenderingPass(r)
	{}

	BufferHandle& getAverageLuminanceBuffer()
	{
		return m_luminanceBuff;
	}

	ANKI_USE_RESULT Error create(const ConfigSet& initializer);

	void run(CommandBufferHandle& cmdb);

private:
	ShaderResourcePointer m_luminanceShader;
	PipelineHandle m_luminancePpline;
	BufferHandle m_luminanceBuff;
};
/// @}

} // end namespace anki

#endif
