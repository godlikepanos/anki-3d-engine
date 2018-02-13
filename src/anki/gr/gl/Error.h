// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/gl/Common.h>

namespace anki
{

/// @addtogroup opengl
/// @{

// Enable the exception on debug. Calling glGetError calls serialization

#if ANKI_EXTRA_CHECKS

/// The function exits if there is an OpenGL error. Use it with the
/// ANKI_CHECK_GL_ERROR macro
void glConditionalCheckError(const char* file, int line, const char* func);

#	define ANKI_CHECK_GL_ERROR() glConditionalCheckError(ANKI_FILE, __LINE__, ANKI_FUNC)
#else
#	define ANKI_CHECK_GL_ERROR() ((void)0)
#endif
/// @}

} // end namespace anki
