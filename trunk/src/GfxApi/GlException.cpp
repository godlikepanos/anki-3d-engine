#include "GlException.h"
#include "Util/Exception.h"
#include "Util/Assert.h"
#include "Core/Logger.h"
#include "Core/Globals.h"
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
	ERROR("OpenGL exception: " << glerr << " @ " << file << ":" << line <<
		" " << func << ")");
	ASSERT(0);
#endif
}
