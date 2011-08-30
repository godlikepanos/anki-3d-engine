#include "GlException.h"
#include "util/Exception.h"
#include "util/Assert.h"
#include "core/Logger.h"
#include "core/Globals.h"
#include <string>
#include <GL/glew.h>


void glConditionalThrowException(const char* file, int line, const char* func)
{
	GLenum errId = glGetError();
	if(errId == GL_NO_ERROR)
	{
		return;
	}

	const char* glerr = reinterpret_cast<const char*>(gluErrorString(errId));
#if defined(NDEBUG)
	throw Exception(std::string("OpenGL exception: ") + glerr, file, line,
		func);
#else
	ERROR("(" << file << ":" << line <<
		" " << func << ") GL Error: " << glerr);
	ASSERT(0);
#endif
}
