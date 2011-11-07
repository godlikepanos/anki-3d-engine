#ifndef ANKI_GL_BUFFER_OBJECT_H
#define ANKI_GL_BUFFER_OBJECT_H

#include <GL/glew.h>
#include "anki/util/Assert.h"
#include "anki/util/StdTypes.h"


namespace anki {


/// A wrapper for OpenGL buffer objects (vertex arrays, texture buffers etc)
/// to prevent us from making idiotic errors
class BufferObject
{
	public:
		BufferObject(): glId(0) {}

		/// Default constructor @see create
		BufferObject(GLenum target, uint sizeInBytes,
			const void* dataPtr, GLenum usage);

		/// It deletes the BO from the GL context
		virtual ~BufferObject();

		/// @name Accessors
		/// @{
		uint getGlId() const;
		GLenum getBufferTarget() const;
		GLenum getBufferUsage() const;
		size_t getSizeInBytes() const;
		/// @]

		/// Bind BO
		void bind() const;

		/// Unbind BO
		void unbind() const;

		/// Creates a new BO with the given parameters and checks if everything
		/// went OK. Throws exception if fails
		/// @param target Depends on the BO
		/// @param sizeInBytes The size of the buffer that we will allocate in
		/// bytes
		/// @param dataPtr Points to the data buffer to copy to the VGA memory.
		/// Put NULL if you want just to allocate memory
		/// @param usage It should be: GL_STREAM_DRAW or GL_STATIC_DRAW or
		/// GL_DYNAMIC_DRAW only!!!!!!!!!
		/// @exception Exception
		void create(GLenum target, uint sizeInBytes, const void* dataPtr,
			GLenum usage);

		/// Delete the BO
		void deleteBuff();

		/// Write data to buffer. This means that maps the BO to local memory,
		/// writes the local memory and unmaps it. Throws exception if the
		/// given size and the BO size are not equal. It throws an exception if
		/// the usage is GL_STATIC_DRAW
		/// @param[in] buff The buffer to copy to BO
		void write(void* buff);

		/// The same as the other write but it maps only a subset of the data
		/// @param[in] buff The buffer to copy to BO
		/// @param[in] offset The offset
		/// @param[in] size The size in bytes we want to write
		void write(void* buff, size_t offset, size_t size);

		/// If created is run successfully it returns true
		bool isCreated() const {return glId != 0;}

	private:
		uint glId; ///< The OpenGL id of the BO

		/// Used in glBindBuffer(target, glId) and its for easy access so we
		/// wont have to query the GL driver. Its the type of the buffer eg
		/// GL_TEXTURE_BUFFER or GL_ELEMENT_ARRAY_BUFFER etc
		GLenum target;

		GLenum usage; ///< GL_STREAM_DRAW or GL_STATIC_DRAW or GL_DYNAMIC_DRAW
		size_t sizeInBytes; ///< The size of the buffer
};


//==============================================================================
// Inlines                                                                     =
//==============================================================================

inline BufferObject::BufferObject(GLenum target, uint sizeInBytes,
	const void* dataPtr, GLenum usage)
:	glId(0)
{
	create(target, sizeInBytes, dataPtr, usage);
}


inline uint BufferObject::getGlId() const
{
	ANKI_ASSERT(isCreated());
	return glId;
}


inline GLenum BufferObject::getBufferTarget() const
{
	ANKI_ASSERT(isCreated());
	return target;
}


inline GLenum BufferObject::getBufferUsage() const
{
	ANKI_ASSERT(isCreated());
	return usage;
}


inline size_t BufferObject::getSizeInBytes() const
{
	ANKI_ASSERT(isCreated());
	return sizeInBytes;
}


inline void BufferObject::bind() const
{
	ANKI_ASSERT(isCreated());
	glBindBuffer(target, glId);
}


inline void BufferObject::unbind() const
{
	ANKI_ASSERT(isCreated());
	glBindBuffer(target, 0);
}


inline void BufferObject::deleteBuff()
{
	ANKI_ASSERT(isCreated());
	glDeleteBuffers(1, &glId);
	glId = 0;
}


} // end namespace


#endif
