// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_BUFFER_H
#define ANKI_GL_GL_BUFFER_H

#include "anki/gl/GlObject.h"

namespace anki {

/// @addtogroup opengl_private
/// @{
	
/// A wrapper for OpenGL buffer objects (vertex arrays, texture buffers etc)
/// to prevent us from making idiotic errors. It's storage immutable
class GlBuffer: public GlObject
{
public:
	typedef GlObject Base;

	/// @name Constructors/Destructor
	/// @{

	/// Default
	GlBuffer()
	{}

	/// Creates a new BO with the given parameters and checks if everything
	/// went OK. Throws exception if fails
	/// @param target Depends on the BO
	/// @param sizeInBytes The size of the buffer that we will allocate in bytes
	/// @param dataPtr Points to the data buffer to copy to the VGA memory.
	///		   Put NULL if you want just to allocate memory
	/// @param flags GL access flags
	GlBuffer(GLenum target, U32 sizeInBytes, const void* dataPtr,
		GLbitfield flags)
	{
		create(target, sizeInBytes, dataPtr, flags);
	}

	/// Move
	GlBuffer(GlBuffer&& b)
	{
		*this = std::move(b);
	}

	/// It deletes the BO
	~GlBuffer()
	{
		destroy();
	}
	/// @}

	/// Move
	GlBuffer& operator=(GlBuffer&& b);

	/// @name Accessors
	/// @{
	GLenum getTarget() const
	{
		ANKI_ASSERT(isCreated());
		return m_target;
	}

	void setTarget(GLenum target)
	{
		ANKI_ASSERT(isCreated());
		unbind(); // Unbind from the previous target
		m_target = target;
	}

	U32 getSize() const
	{
		ANKI_ASSERT(isCreated());
		return m_size;
	}

	/// Return the prersistent mapped address
	void* getPersistentMappingAddress()
	{
		ANKI_ASSERT(isCreated());
		ANKI_ASSERT(m_persistentMapping);
		return m_persistentMapping;
	}
	/// @}

	/// Bind
	void bind() const
	{
		ANKI_ASSERT(isCreated());
		glBindBuffer(m_target, m_glName);
	}

	/// Unbind BO
	void unbind() const
	{
		ANKI_ASSERT(isCreated());
		bindDefault(m_target);
	}

	/// Bind the default to a target
	static void bindDefault(GLenum target)
	{
		glBindBuffer(target, 0);
	}

	/// Write data to buffer. 
	/// @param[in] buff The buffer to copy to BO
	void write(void* buff)
	{
		write(buff, 0, m_size);
	}

	/// The same as the other write but it maps only a subset of the data
	/// @param[in] buff The buffer to copy to BO
	/// @param[in] offset The offset
	/// @param[in] size The size in bytes we want to write
	void write(void* buff, U32 offset, U32 size);

	/// Set the binding for this buffer
	void setBinding(GLuint binding) const;

	/// Set the binding point of this buffer with range
	void setBindingRange(GLuint binding, PtrSize offset, PtrSize size) const;

private:
	GLenum m_target; ///< GL_TEXTURE_BUFFER or GL_ELEMENT_ARRAY_BUFFER etc
	U32 m_size; ///< The size of the buffer
	void* m_persistentMapping = nullptr;

	/// Create
	void create(GLenum target, U32 sizeInBytes, const void* dataPtr,
		GLbitfield flags);

	/// Delete the BO
	void destroy();
};

/// @}

} // end namespace anki

#endif
