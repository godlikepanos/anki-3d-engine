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

	public:
		/// Default constructor @see create
		BufferObject(GLenum target, uint sizeInBytes, const void* dataPtr, GLenum usage, Object* parent = NULL);

		/// It deletes the BO from the GL context
		virtual ~BufferObject() {deleteBuff();}

		/// Bind BO. Throws exception if BO is not created
		/// @exception Exception
		void bind() const;

		/// Unbind BO. Throws exception if BO is not created
		/// @exception Exception
		void unbind() const;

	private:
		/// Creates a new BO with the given parameters and checks if everything went OK. Throws exception if fails
		/// @param target Depends on the BO
		/// @param sizeInBytes The size of the buffer that we will allocate in bytes
		/// @param dataPtr Points to the data buffer to copy to the VGA memory. Put NULL if you want just to allocate memory
		/// @param usage It should be: GL_STREAM_DRAW or GL_STATIC_DRAW or GL_DYNAMIC_DRAW only!!!!!!!!!
		/// @exception Exception
		void create(GLenum target, uint sizeInBytes, const void* dataPtr, GLenum usage);

		/// Delete the BO
		/// @exception Exception
		void deleteBuff();
};


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================

inline BufferObject::BufferObject(GLenum target, uint sizeInBytes, const void* dataPtr, GLenum usage, Object* parent):
	Object(parent)
{
	create(target, sizeInBytes, dataPtr, usage);
}


inline void BufferObject::bind() const
{
	glBindBuffer(target, glId);
}


inline void BufferObject::unbind() const
{
	glBindBuffer(target, 0);
}


inline void BufferObject::deleteBuff()
{
	glDeleteBuffers(1, &glId);
	glId = 0;
}


#endif
