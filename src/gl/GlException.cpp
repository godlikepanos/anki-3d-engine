#include "anki/gl/GlException.h"
#include "anki/gl/Gl.h"
#include "anki/util/Exception.h"
#include <string>

namespace anki {

void glConditionalThrowException(const char* file, int line, const char* func)
{
	GLenum errId = glGetError();
	if(errId == GL_NO_ERROR)
	{
		return;
	}

	const errStr = nullptr;

	switch(errId)
	{
	case GL_INVALID_ENUM:
		errStr = "GL_INVALID_ENUM";
		break;
	case GL_INVALID_VALUE:
		errStr = "GL_INVALID_VALUE";
		break;
	case GL_INVALID_OPERATION:
		errStr = "GL_INVALID_OPERATION";
		break;
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		errStr = "GL_INVALID_FRAMEBUFFER_OPERATION";
		break;
	case GL_OUT_OF_MEMORY:
		errStr = "GL_OUT_OF_MEMORY";
		break;
	default:
		errStr = "unknown";
	};

	std::string err = std::string("OpenGL exception: ") + errStr;
	throw Exception(err.c_str(), file, line, func);
}

} // end namespace
