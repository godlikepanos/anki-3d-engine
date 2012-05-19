#include "anki/gl/GlException.h"
#include "anki/util/Exception.h"
#include <string>
#include <GL/glew.h>

namespace anki {

void glConditionalThrowException(const char* file, int line, const char* func)
{
	GLenum errId = glGetError();
	if(errId == GL_NO_ERROR)
	{
		return;
	}

	const char* glerr = reinterpret_cast<const char*>(gluErrorString(errId));
	std::string err = std::string("OpenGL exception: ") + glerr;
	throw Exception(err.c_str(), file, line, func);
}

} // end namespace
