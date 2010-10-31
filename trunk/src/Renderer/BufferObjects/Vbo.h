#ifndef VBO_H
#define VBO_H

#include "BufferObject.h"


/// This is a wrapper for Vertex Buffer Objects to prevent us from making idiotic errors
class Vbo: public BufferObject
{
	public:
		/// It adds an extra check over @ref BufferObject::create. @see @ref BufferObject::create
		void create(GLenum target_, uint sizeInBytes, const void* dataPtr, GLenum usage_);

		/// Unbinds all VBOs, meaning both GL_ARRAY_BUFFER and GL_ELEMENT_ARRAY_BUFFER targets
		static void unbindAllTargets();
};


inline void Vbo::create(GLenum target_, uint sizeInBytes, const void* dataPtr, GLenum usage_)
{
	RASSERT_THROW_EXCEPTION(target_ != GL_ARRAY_BUFFER && target_ != GL_ELEMENT_ARRAY_BUFFER); // unacceptable target_
	BufferObject::create(target_, sizeInBytes, dataPtr, usage_);
}


inline void Vbo::unbindAllTargets()
{
	glBindBufferARB(GL_ARRAY_BUFFER, 0);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
}


#endif
