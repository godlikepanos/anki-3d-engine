// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_PIPELINE_COMMON_H
#define ANKI_GR_PIPELINE_COMMON_H

#include "anki/gr/Common.h"
#include "anki/gr/ShaderHandle.h"

namespace anki {

/// @addtogroup graphics
/// @{

/// Pipeline initializer.
struct PipelineInitializer
{
	Array<ShaderHandle, 5> m_graphicsShaders;
	ShaderHandle m_computeShader;
};
/// @}

} // end namespace anki

#endif
