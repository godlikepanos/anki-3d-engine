#include "anki/gl/GlException.h"
#include "anki/util/Exception.h"
#include "anki/util/Assert.h"
#include "anki/core/Logger.h"
#include "anki/core/Globals.h"
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
#if defined(NDEBUG)
	throw Exception(std::string("OpenGL exception: ") + glerr, file, line,
		func);
#else
	ANKI_ERROR("(" << file << ":" << line <<
		" " << func << ") GL Error: " << glerr);
	ANKI_ASSERT(0);
#endif
}


} // end namespace
