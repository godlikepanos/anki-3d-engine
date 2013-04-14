#ifndef ANKI_GL_UBO_H
#define ANKI_GL_UBO_H

#include "anki/gl/BufferObject.h"
#include "anki/util/StdTypes.h"

namespace anki {

/// @addtogroup OpenGL
/// @{
	
/// Uniform buffer object
class Ubo: public BufferObject
{
public:
	void create(PtrSize size, void* data)
	{
		// XXX GL_MAX_UNIFORM_BLOCK_SIZE
		GLint64 maxBufferSize;
		glGetInteger64v(GL_MAX_UNIFORM_BLOCK_SIZE, &maxBufferSize);
		BufferObject::create(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
	}
};
/// @}

} // end namespace anki

#endif
