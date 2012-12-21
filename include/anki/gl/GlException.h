#ifndef ANKI_GL_GL_EXCEPTION_H
#define ANKI_GL_GL_EXCEPTION_H

#include "anki/Config.h"

namespace anki {

/// @addtogroup OpenGL
/// @{

/// The function throws an exception if there is an OpenGL error. Use it with
/// the ANKI_CHECK_GL_ERROR macro
void glConditionalThrowException(const char* file, int line, const char* func);

#if ANKI_DEBUG
#	define ANKI_CHECK_GL_ERROR() \
		glConditionalThrowException(ANKI_FILE, __LINE__, ANKI_FUNC)
#else
#	define ANKI_CHECK_GL_ERROR() ((void)0)
#endif

/// @}

} // end namespace

#endif
