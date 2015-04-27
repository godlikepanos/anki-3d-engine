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

	ANKI_USE_RESULT Error create(const ConfigSet& initializer);

	ANKI_USE_RESULT Error run(CommandBufferHandle& cmdb);

private:
	ShaderResourcePointer m_avgLuminanceShader;
	PipelineHandle m_avgLuminancePpline;
	BufferHandle m_luminanceBuff;
};

} // end namespace anki

#endif
