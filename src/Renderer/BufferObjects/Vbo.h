#ifndef _VBO_H_
#define _VBO_H_

#include "Common.h"
#include "BufferObject.h"

/**
 * This is a wrapper for Vertex Buffer Objects to prevent us from making idiotic errors
 */
class Vbo: public BufferObject
{
	public:
		/**
		 * It adds an extra check over @ref BufferObject::create. See @ref BufferObject::create for details
		 */
		bool create(GLenum target_, uint sizeInBytes, const void* dataPtr, GLenum usage_);

		/**
		 * Unbinds all VBOs, meaning both GL_ARRAY_BUFFER and GL_ELEMENT_ARRAY_BUFFER targets
		 */
		static void unbindAllTargets();
};


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================

inline bool Vbo::create(GLenum target_, uint sizeInBytes, const void* dataPtr, GLenum usage_)
{
	DEBUG_ERR(target_!=GL_ARRAY_BUFFER && target_!=GL_ELEMENT_ARRAY_BUFFER); // unacceptable target_

	return BufferObject::create(target_, sizeInBytes, dataPtr, usage_);
}


inline void Vbo::unbindAllTargets()
{
	glBindBufferARB(GL_ARRAY_BUFFER, 0);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
}

#endif
