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
		BufferObject::create(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
	}
};
/// @}

} // end namespace anki

#endif
