// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_GL_ERROR_H
#define ANKI_GR_GL_ERROR_H

#include "anki/gr/Common.h"

namespace anki {

/// @addtogroup opengl_private
/// @{

// Enable the exception on debug. Calling glGetError calls serialization

#if ANKI_DEBUG

/// The function exits if there is an OpenGL error. Use it with the 
/// ANKI_CHECK_GL_ERROR macro
void glConditionalCheckError(const char* file, int line, const char* func);

#	define ANKI_CHECK_GL_ERROR() \
		glConditionalCheckError(ANKI_FILE, __LINE__, ANKI_FUNC)
#else
#	define ANKI_CHECK_GL_ERROR() ((void)0)
#endif
/// @}

} // end namespace anki

#endif
