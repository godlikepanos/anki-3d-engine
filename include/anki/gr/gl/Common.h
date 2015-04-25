// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/Common.h"

namespace anki {

/// @addtogroup opengl
/// @{

/// Converter.
GLenum convertCompareOperation(CompareOperation in);

void convertFilter(SamplingFilter minMagFilter, SamplingFilter mipFilter,
	GLenum& minFilter, GLenum& magFilter);
/// @}

} // end namespace anki

