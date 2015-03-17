// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/gl/BufferImpl.h"
#include "anki/gr/gl/Error.h"
#include "anki/util/Logger.h"
#include <cstring>
#include <cmath>

namespace anki {

//==============================================================================
void BufferImpl::destroy()
{
	if(isCreated())
	{
		glDeleteBuffers(1, &m_glName);
		m_glName = 0;
	}
}

//==============================================================================
Error BufferImpl::create(GLenum target, const void* dataPtr, 
	U32 sizeInBytes, GLbitfield flags)
{
	ANKI_ASSERT(!isCreated());

	if(target == GL_UNIFORM_BUFFER)
	{
		GLint64 maxBufferSize;
		glGetInteger64v(GL_MAX_UNIFORM_BLOCK_SIZE, &maxBufferSize);

		if(sizeInBytes > 16384)
		{
			ANKI_LOGW("The size (%u) of the uniform buffer is greater "
				"than the spec's min", sizeInBytes);
		} 
		else if(sizeInBytes > (PtrSize)maxBufferSize)
		{
			ANKI_LOGW("The size (%u) of the uniform buffer is greater "
				"than the implementation's min (%u)", sizeInBytes, 
				maxBufferSize);
		}
	}
	else if(target == GL_SHADER_STORAGE_BUFFER)
	{
		GLint64 maxBufferSize;
		glGetInteger64v(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &maxBufferSize);

		if(sizeInBytes > pow(2, 24))
		{
			ANKI_LOGW("The size (%u) of the uniform buffer is greater "
				"than the spec's min", sizeInBytes);
		} 
		else if(sizeInBytes > (PtrSize)maxBufferSize)
		{
			ANKI_LOGW("The size (%u) of the shader storage buffer is greater "
				"than the implementation's min (%u)", sizeInBytes, 
				maxBufferSize);
		}
	}

	m_target = target;
	m_size = sizeInBytes;

	ANKI_ASSERT(m_size > 0 && "Unacceptable size");

	// Create
	glGenBuffers(1, &m_glName);

	glBindBuffer(m_target, m_glName);
	glBufferStorage(m_target, m_size, dataPtr, flags);

	// Map if needed
	if((flags & GL_MAP_PERSISTENT_BIT) && (flags & GL_MAP_COHERENT_BIT))
	{
		const GLbitfield mapbits = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT 
			| GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

		m_persistentMapping = 
			glMapBufferRange(m_target, 0, sizeInBytes, flags & mapbits);
		ANKI_ASSERT(m_persistentMapping != nullptr);
	}

	return ErrorCode::NONE;
}

//==============================================================================
void BufferImpl::write(void* buff, U32 offset, U32 size)
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(offset + size <= size);

	bind();
	glBufferSubData(m_target, offset, size, buff);
}

//==============================================================================
void BufferImpl::setBinding(GLuint binding) const
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(m_target == GL_SHADER_STORAGE_BUFFER 
		|| m_target == GL_UNIFORM_BUFFER);
	glBindBufferBase(m_target, binding, m_glName);
}

//==============================================================================
void BufferImpl::setBindingRange(
	GLuint binding, PtrSize offset, PtrSize size) const
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(offset + size <= m_size);
	ANKI_ASSERT(size > 0);
	ANKI_ASSERT(m_target == GL_SHADER_STORAGE_BUFFER
		|| m_target == GL_UNIFORM_BUFFER);

	glBindBufferRange(m_target, binding, m_glName, offset, size);
}

} // end namespace anki
