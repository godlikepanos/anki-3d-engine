#ifndef BUFFER_OBJECT_H
#define BUFFER_OBJECT_H

#include <GL/glew.h>
#include "Exception.h"
#include "StdTypes.h"
#include "Object.h"
#include "Properties.h"


/// A wrapper for OpenGL buffer objects (vertex arrays, texture buffers etc) to prevent us from making idiotic errors
class BufferObject: public Object
{
	PROPERTY_R(uint, glId, getGlId) ///< The OpenGL id of the BO

	/// Used in glBindBuffer(target, glId) and its for easy access so we wont have to query the GL driver. Its the type
	/// of the buffer eg GL_TEXTURE_BUFFER or GL_ELEMENT_ARRAY_BUFFER etc
	PROPERTY_R(GLenum, target, getBufferTarget)

	PROPERTY_R(GLenum, usage, getBufferUsage) ///< GL_STREAM_DRAW or GL_STATIC_DRAW or GL_DYNAMIC_DRAW

	PROPERTY_R(size_t, sizeInBytes, getSizeInBytes)

	public:
		/// Default constructor @see create
		BufferObject(GLenum target, uint sizeInBytes, const void* dataPtr, GLenum usage, Object* parent = NULL);

		/// It deletes the BO from the GL context
		virtual ~BufferObject() {deleteBuff();}

		/// Bind BO
		void bind() const {glBindBuffer(target, glId);}

		/// Unbind BO
		void unbind() const {glBindBuffer(target, 0);}

		/// Write data to buffer. This means that maps the BO to local memory, writes the local memory and unmaps it.
		/// Throws exception if the given size and the BO size are not equal. It throws an exception if the usage is
		/// GL_STATIC_DRAW
		/// @param[in] buff The buffer to copy to BO
		void write(void* buff);

		/// The same as the other write but it maps only a subset of the data
		/// @param[in] buff The buffer to copy to BO
		/// @param[in] offset The offset
		/// @param[in] size The size in bytes we want to write
		void write(void* buff, size_t offset, size_t size);

	private:
		/// Creates a new BO with the given parameters and checks if everything went OK. Throws exception if fails
		/// @param target Depends on the BO
		/// @param sizeInBytes The size of the buffer that we will allocate in bytes
		/// @param dataPtr Points to the data buffer to copy to the VGA memory. Put NULL if you want just to allocate memory
		/// @param usage It should be: GL_STREAM_DRAW or GL_STATIC_DRAW or GL_DYNAMIC_DRAW only!!!!!!!!!
		/// @exception Exception
		void create(GLenum target, uint sizeInBytes, const void* dataPtr, GLenum usage);

		/// Delete the BO
		void deleteBuff();
};


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================

inline BufferObject::BufferObject(GLenum target, uint sizeInBytes, const void* dataPtr, GLenum usage, Object* parent):
	Object(parent),
	glId(0)
{
	create(target, sizeInBytes, dataPtr, usage);
}


inline void BufferObject::deleteBuff()
{
	if(glId != 0)
	{
		glDeleteBuffers(1, &glId);
	}
	glId = 0;
}


#endif
