#ifndef ANKI_GL_BUFFER_OBJECT_H
#define ANKI_GL_BUFFER_OBJECT_H

#include "anki/gl/GlObject.h"

namespace anki {

/// @addtogroup OpenGL
/// @{
	
/// A wrapper for OpenGL buffer objects (vertex arrays, texture buffers etc)
/// to prevent us from making idiotic errors
class BufferObject: public GlObject
{
public:
	typedef GlObject Base;

	/// @name Constructors/Destructor
	/// @{

	/// Default
	BufferObject()
	{}

	/// Move
	BufferObject(BufferObject&& b)	
	{
		*this = std::move(b);
	}

	/// It deletes the BO
	~BufferObject()
	{
		destroy();
	}
	/// @}

	/// Move
	BufferObject& operator=(BufferObject&& b);

	/// @name Accessors
	/// @{
	GLenum getTarget() const
	{
		ANKI_ASSERT(isCreated());
		return target;
	}

	void setTarget(GLenum target_)
	{
		ANKI_ASSERT(isCreated());
		target = target_;
	}

	GLenum getUsage() const
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

	/// Bind
	void bind() const
	{
		ANKI_ASSERT(isCreated());
		glBindBuffer(target, getGlId());
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
	/// @param objectCount The number of objects
	void create(GLenum target, U32 sizeInBytes, const void* dataPtr,
		GLenum usage, U objectCount = SINGLE_OBJECT);

	/// Delete the BO
	void destroy();

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

	/// Read the containts of the buffer
	/// @param[out] outBuff The buffer to copy from BO
	void read(void* outBuff)
	{
		read(outBuff, 0, sizeInBytes);
	}

	/// Read the containts of the buffer
	/// @param[in] outBuff The buffer to copy from BO
	/// @param[in] offset The offset
	/// @param[in] size The size in bytes we want to write
	void read(void* buff, U32 offset, U32 size);

	/// Map part of the buffer
	void* map(U32 offset, U32 length, GLuint flags);

	/// Map the entire buffer
	void* map(GLuint flags)
	{
		return map(0, sizeInBytes, flags);
	}

	/// Unmap buffer
	void unmap();

	/// Set the binding for this buffer
	void setBinding(GLuint binding) const;

	/// Set the binding point of this buffer with range
	void setBindingRange(GLuint binding, PtrSize offset, PtrSize size) const;

	/// Return GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT
	static PtrSize getUniformBufferOffsetAlignment()
	{
		GLint64 offsetAlignment;
		glGetInteger64v(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &offsetAlignment);
		return offsetAlignment;
	}

private:
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

/// This is a wrapper for Vertex Buffer Objects to prevent us from making
/// idiotic errors
class Vbo: public BufferObject
{
public:
	/// The same as BufferObject::create but it only accepts
	/// GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER in target
	/// @see BufferObject::create
	void create(GLenum target, PtrSize sizeInBytes, const void* dataPtr,
		GLenum usage, U objectCount = SINGLE_OBJECT)
	{
		// unacceptable target_
		ANKI_ASSERT(target == GL_ARRAY_BUFFER
			|| target == GL_ELEMENT_ARRAY_BUFFER);
		BufferObject::create(target, sizeInBytes, dataPtr, usage, objectCount);
	}

	/// Unbinds all VBOs, meaning both GL_ARRAY_BUFFER and
	/// GL_ELEMENT_ARRAY_BUFFER targets
	static void unbindAllTargets()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
};

/// Uniform buffer object
class Ubo: public BufferObject
{
public:
	/// Create a UBO
	void create(PtrSize size, const void* data, U objectCount = SINGLE_OBJECT)
	{
		GLint64 maxBufferSize;
		glGetInteger64v(GL_MAX_UNIFORM_BLOCK_SIZE, &maxBufferSize);
		ANKI_ASSERT(size <= (PtrSize)maxBufferSize);

		BufferObject::create(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW,
			objectCount);
	}
};

/// Pixel buffer object
class Pbo: public BufferObject
{
public:
	/// Create a PBO
	void create(GLenum target, PtrSize size, const void* data, 
		U objectCount = SINGLE_OBJECT)
	{
		ANKI_ASSERT(target == GL_PIXEL_PACK_BUFFER 
			|| target == GL_PIXEL_UNPACK_BUFFER);

		GLenum pboUsage = (target == GL_PIXEL_PACK_BUFFER) 
			? GL_DYNAMIC_READ : GL_DYNAMIC_DRAW;

		BufferObject::create(target, size, data, pboUsage, objectCount);
	}
};

/// @}

} // end namespace anki

#endif
