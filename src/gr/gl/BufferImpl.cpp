// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/BufferImpl.h>
#include <anki/util/Logger.h>
#include <cmath>

namespace anki
{

//==============================================================================
void BufferImpl::create(
	PtrSize size, BufferUsageBit usage, BufferAccessBit access)
{
	ANKI_ASSERT(!isCreated());
	m_usage = usage;
	m_access = access;

	///
	// Check size
	//

	ANKI_ASSERT(size > 0 && "Unacceptable size");

	// This is a guess, not very important since DSA doesn't care about it on
	// creation
	m_target = GL_ARRAY_BUFFER;

	if((usage & BufferUsageBit::UNIFORM) != BufferUsageBit::NONE)
	{
		GLint64 maxBufferSize;
		glGetInteger64v(GL_MAX_UNIFORM_BLOCK_SIZE, &maxBufferSize);

		if(size > 16384)
		{
			ANKI_LOGW("The size (%u) of the uniform buffer is greater "
					  "than the spec's min",
				size);
		}
		else if(size > PtrSize(maxBufferSize))
		{
			ANKI_LOGW("The size (%u) of the uniform buffer is greater "
					  "than the implementation's min (%u)",
				size,
				maxBufferSize);
		}

		m_target = GL_UNIFORM_BUFFER;
	}

	if((usage & BufferUsageBit::STORAGE) != BufferUsageBit::NONE)
	{
		GLint64 maxBufferSize;
		glGetInteger64v(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &maxBufferSize);

		if(size > pow(2, 24))
		{
			ANKI_LOGW("The size (%u) of the uniform buffer is greater "
					  "than the spec's min",
				size);
		}
		else if(size > PtrSize(maxBufferSize))
		{
			ANKI_LOGW("The size (%u) of the shader storage buffer is greater "
					  "than the implementation's min (%u)",
				size,
				maxBufferSize);
		}

		m_target = GL_SHADER_STORAGE_BUFFER;
	}

	m_size = size;

	//
	// Determine the creation flags
	//
	GLbitfield flags = 0;
	Bool shouldMap = false;
	if((access & BufferAccessBit::CLIENT_WRITE) != BufferAccessBit::NONE)
	{
		flags |= GL_DYNAMIC_STORAGE_BIT;
	}

	if((access & BufferAccessBit::CLIENT_MAP_WRITE) != BufferAccessBit::NONE)
	{
		flags |= GL_MAP_WRITE_BIT;
		flags |= GL_MAP_PERSISTENT_BIT;
		flags |= GL_MAP_COHERENT_BIT;

		shouldMap = true;
	}

	if((access & BufferAccessBit::CLIENT_MAP_READ) != BufferAccessBit::NONE)
	{
		flags |= GL_MAP_READ_BIT;
		flags |= GL_MAP_PERSISTENT_BIT;
		flags |= GL_MAP_COHERENT_BIT;

		shouldMap = true;
	}

	//
	// Create
	//
	glGenBuffers(1, &m_glName);
	glBindBuffer(m_target, m_glName);
	glBufferStorage(m_target, size, nullptr, flags);

	//
	// Map
	//
	if(shouldMap)
	{
		const GLbitfield MAP_BITS = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT
			| GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

		m_persistentMapping =
			glMapBufferRange(m_target, 0, size, flags & MAP_BITS);
		ANKI_ASSERT(m_persistentMapping != nullptr);
	}
}

} // end namespace anki
