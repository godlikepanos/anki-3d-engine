#ifndef ANKI_GL_DRAWCALL_H
#define ANKI_GL_DRAWCALL_H

#include "anki/gl/Ogl.h"
#include "anki/util/StdTypes.h"
#include "anki/util/Array.h"

namespace anki {

/// @addtogroup OpenGL
/// @{

/// A GL drawcall
struct Drawcall
{
	GLenum primitiveType;
	
	/// Type of the indices. If zero then draw with glDrawArraysXXX
	GLenum indicesType; 

	U32 instancesCount;

	/// Used in glMultiDrawXXX
	U32 drawCount;

	/// The indices or elements
	union
	{
		U32 count;
		Array<U32, ANKI_MAX_MULTIDRAW_PRIMITIVES> countArray;
	};

	union
	{
		PtrSize offset;
		Array<PtrSize, ANKI_MAX_MULTIDRAW_PRIMITIVES> offsetArray;
	};

	Drawcall();

	/// Execute the drawcall
	void enque();
};
/// @}

static_assert(sizeof(GLsizei) == sizeof(U32), "Wrong assumption");

} // end namespace anki

#endif
