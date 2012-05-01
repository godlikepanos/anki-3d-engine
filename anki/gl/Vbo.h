#ifndef ANKI_GL_VBO_H
#define ANKI_GL_VBO_H

#include "anki/gl/BufferObject.h"


namespace anki {


/// This is a wrapper for Vertex Buffer Objects to prevent us from making
/// idiotic errors
class Vbo: public BufferObject
{
public:
	Vbo()
	{}

	/// Adds an extra check in target_ @see BufferObject::BufferObject
	Vbo(GLenum target_, uint sizeInBytes_, const void* dataPtr_,
		GLenum usage_)
	{
		create(target_, sizeInBytes_, dataPtr_, usage_);
	}

	/// Unbinds all VBOs, meaning both GL_ARRAY_BUFFER and
	/// GL_ELEMENT_ARRAY_BUFFER targets
	static void unbindAllTargets()
	{
		glBindBufferARB(GL_ARRAY_BUFFER, 0);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	/// The same as BufferObject::create but it only accepts
	/// GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER in target
	/// @see BufferObject::create
	void create(GLenum target, uint sizeInBytes, const void* dataPtr,
		GLenum usage)
	{
		// unacceptable target_
		ANKI_ASSERT(target == GL_ARRAY_BUFFER
			|| target == GL_ELEMENT_ARRAY_BUFFER);
		BufferObject::create(target, sizeInBytes, dataPtr, usage);
	}
};


} // end namespace


#endif
