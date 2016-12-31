// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/BufferImpl.h>
#include <anki/util/Logger.h>
#include <cmath>

namespace anki
{

void BufferImpl::init(PtrSize size, BufferUsageBit usage, BufferMapAccessBit access)
{
	ANKI_ASSERT(!isCreated());
	m_usage = usage;
	m_access = access;

	///
	// Check size
	//

	ANKI_ASSERT(size > 0 && "Unacceptable size");

	// Align size to satisfy BufferImpl::fill
	alignRoundUp(4, size);

	// This is a guess, not very important since DSA doesn't care about it on creation
	m_target = GL_ARRAY_BUFFER;

	if((usage & BufferUsageBit::UNIFORM_ALL) != BufferUsageBit::NONE)
	{
		m_target = GL_UNIFORM_BUFFER;
	}

	if((usage & BufferUsageBit::STORAGE_ALL) != BufferUsageBit::NONE)
	{
		m_target = GL_SHADER_STORAGE_BUFFER;
	}

	m_size = size;

	//
	// Determine the creation flags
	//
	GLbitfield flags = 0;
	Bool shouldMap = false;
	if((usage & BufferUsageBit::TRANSFER_ALL) != BufferUsageBit::NONE)
	{
		flags |= GL_DYNAMIC_STORAGE_BIT;
	}

	if((access & BufferMapAccessBit::WRITE) != BufferMapAccessBit::NONE)
	{
		flags |= GL_MAP_WRITE_BIT;
		flags |= GL_MAP_PERSISTENT_BIT;
		flags |= GL_MAP_COHERENT_BIT;

		shouldMap = true;
	}

	if((access & BufferMapAccessBit::READ) != BufferMapAccessBit::NONE)
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
	// Align the size to satisfy BufferImpl::fill
	glBufferStorage(m_target, size, nullptr, flags);

	//
	// Map
	//
	if(shouldMap)
	{
		const GLbitfield MAP_BITS = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

		m_persistentMapping = glMapBufferRange(m_target, 0, size, flags & MAP_BITS);
		ANKI_ASSERT(m_persistentMapping != nullptr);
	}
}

} // end namespace anki
