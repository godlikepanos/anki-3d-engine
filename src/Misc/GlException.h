#ifndef GL_EXCEPTION_H
#define GL_EXCEPTION_H

#include <string>
#include "Exception.h"


/// The function throws an exception if there is an OpenGL error. Use it with the ON_GL_FAIL_THROW_EXCEPTION macro
inline void glConditionalThrowException(const char* file, int line, const char* func)
{
	GLenum errId = glGetError();
	if(errId == GL_NO_ERROR)
	{
		return;
	}

	const char* glerr = reinterpret_cast<const char*>(gluErrorString(errId));
	throw Exception(std::string("OpenGL exception: ") + glerr, file, line, func);
}


#define ON_GL_FAIL_THROW_EXCEPTION() glConditionalThrowException(__FILE__, __LINE__, __func__)


#endif
