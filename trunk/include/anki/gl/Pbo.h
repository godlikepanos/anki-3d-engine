#ifndef ANKI_GL_PBO_H
#define ANKI_GL_PBO_H

#include "anki/gl/BufferObject.h"

namespace anki {

/// @addtogroup OpenGL
/// @{

/// Pixel buffer object
class Pbo: public BufferObject
{
public:
	void create(GLenum target, PtrSize size, void* data)
	{
		ANKI_ASSERT(target == GL_PIXEL_PACK_BUFFER 
			|| target == GL_PIXEL_UNPACK_BUFFER);

		GLenum pboUsage = (target == GL_PIXEL_PACK_BUFFER) 
			? GL_STREAM_READ : GL_STREAM_DRAW;

		BufferObject::create(target, size, data, pboUsage);
	}
};
/// @}

} // end namespace anki

#endif
