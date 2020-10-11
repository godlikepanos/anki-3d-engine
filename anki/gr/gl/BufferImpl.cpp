// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/BufferImpl.h>
#include <anki/util/Logger.h>
#include <cmath>

namespace anki
{

void BufferImpl::init()
{
	ANKI_ASSERT(!isCreated());

	const BufferUsageBit usage = getBufferUsage();
	const BufferMapAccessBit access = getMapAccess();

	// Align size to satisfy BufferImpl::fill
	m_realSize = getAlignedRoundUp(4, getSize());

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
	glBufferStorage(m_target, m_realSize, nullptr, flags);

	//
	// Map
	//
	if(shouldMap)
	{
		const GLbitfield MAP_BITS = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

		m_persistentMapping = glMapBufferRange(m_target, 0, m_size, flags & MAP_BITS);
		ANKI_ASSERT(m_persistentMapping != nullptr);
	}
}

} // end namespace anki
