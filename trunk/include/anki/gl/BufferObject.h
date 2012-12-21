#ifndef ANKI_GL_BUFFER_OBJECT_H
#define ANKI_GL_BUFFER_OBJECT_H

#include "anki/gl/Ogl.h"
#include "anki/util/Assert.h"
#include "anki/util/NonCopyable.h"
#include "anki/util/StdTypes.h"

namespace anki {

/// @addtogroup OpenGL
/// @{
	
/// A wrapper for OpenGL buffer objects (vertex arrays, texture buffers etc)
/// to prevent us from making idiotic errors
class BufferObject: public NonCopyable
{
public:
	/// @name Constructors/Destructor
	/// @{

	/// Default
	BufferObject()
	{}

	/// Move
	BufferObject(BufferObject&& b)
		: glId(b.glId), target(b.target), usage(b.usage), 
			sizeInBytes(b.sizeInBytes)
#if ANKI_DEBUG
			, mapped(b.mapped)
#endif
	{
		b.glId = 0;
	}

	/// @see create
	BufferObject(GLenum target, U32 sizeInBytes,
		const void* dataPtr, GLenum usage)
	{
		create(target, sizeInBytes, dataPtr, usage);
	}

	/// It deletes the BO
	~BufferObject();
	/// @}

	/// @name Accessors
	/// @{
	GLuint getGlId() const
	{
		ANKI_ASSERT(isCreated());
		return glId;
	}

	GLenum getBufferTarget() const
	{
		ANKI_ASSERT(isCreated());
		return target;
	}

	GLenum getBufferUsage() const
	{
		ANKI_ASSERT(isCreated());
		return usage;
	}

	U32 getSizeInBytes() const
	{
		ANKI_ASSERT(isCreated());
		return sizeInBytes;
	}
	/// @}

	/// Bind BO
	void bind() const
	{
		ANKI_ASSERT(isCreated());
		glBindBuffer(target, glId);
	}

	/// Unbind BO
	void unbind() const
	{
		ANKI_ASSERT(isCreated());
		glBindBuffer(target, 0);
	}

	/// Creates a new BO with the given parameters and checks if everything
	/// went OK. Throws exception if fails
	/// @param target Depends on the BO
	/// @param sizeInBytes The size of the buffer that we will allocate in bytes
	/// @param dataPtr Points to the data buffer to copy to the VGA memory.
	///		   Put NULL if you want just to allocate memory
	/// @param usage It should be: GL_STREAM_DRAW or GL_STATIC_DRAW or
	///		   GL_DYNAMIC_DRAW only!!!!!!!!!
	/// @exception Exception
	void create(GLenum target, U32 sizeInBytes, const void* dataPtr,
		GLenum usage);

	/// Delete the BO
	void destroy()
	{
		ANKI_ASSERT(isCreated());
		unbind();
		glDeleteBuffers(1, &glId);
		glId = 0;
	}

	/// Write data to buffer. This means that maps the BO to local memory,
	/// writes the local memory and unmaps it. Throws exception if the
	/// given size and the BO size are not equal. It throws an exception if
	/// the usage is GL_STATIC_DRAW
	/// @param[in] buff The buffer to copy to BO
	void write(void* buff)
	{
		write(buff, 0, sizeInBytes);
	}

	/// The same as the other write but it maps only a subset of the data
	/// @param[in] buff The buffer to copy to BO
	/// @param[in] offset The offset
	/// @param[in] size The size in bytes we want to write
	void write(void* buff, U32 offset, U32 size);

	/// Map part of the buffer
	void* map(U32 offset, U32 length, GLuint flags);

	/// Map the entire buffer
	void* map(GLuint flags)
	{
		return map(0, sizeInBytes, flags);
	}

	/// Unmap buffer
	void unmap();

	/// If create() is run successfully this returns true
	Bool isCreated() const
	{
		return glId != 0;
	}

	/// Set the binding for this buffer
	void setBinding(GLuint binding) const
	{
		ANKI_ASSERT(target == GL_TRANSFORM_FEEDBACK_BUFFER 
			|| target == GL_UNIFORM_BUFFER);
		bind();
		glBindBufferBase(target, binding, glId);
	}

private:
	GLuint glId = 0; ///< The OpenGL id of the BO

	/// Used in glBindBuffer(target, glId) and its for easy access so we
	/// wont have to query the GL driver. Its the type of the buffer eg
	/// GL_TEXTURE_BUFFER or GL_ELEMENT_ARRAY_BUFFER etc
	GLenum target;

	GLenum usage; ///< GL_STREAM_DRAW or GL_STATIC_DRAW or GL_DYNAMIC_DRAW
	U32 sizeInBytes; ///< The size of the buffer

#if ANKI_DEBUG
	Bool mapped = false; ///< Only in debug
#endif
};
/// @}

} // end namespace anki

#endif
