// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Common.h>

#if ANKI_GL == ANKI_GL_DESKTOP
#	if ANKI_OS == ANKI_OS_WINDOWS && !defined(GLEW_STATIC)
#		define GLEW_STATIC
#	endif
#	include <GL/glew.h>
#	if !defined(ANKI_GLEW_H)
#		error "Wrong GLEW included"
#	endif
#elif ANKI_GL == ANKI_GL_ES
#	include <GLES3/gl3.h>
#else
#	error "See file"
#endif

namespace anki {

// Forward
class GlState;
class RenderingThread;

/// @addtogroup opengl
/// @{

/// Converter.
GLenum convertCompareOperation(CompareOperation in);

void convertFilter(SamplingFilter minMagFilter, SamplingFilter mipFilter,
	GLenum& minFilter, GLenum& magFilter);
/// @}

} // end namespace anki

