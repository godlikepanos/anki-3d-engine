// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/gl/GlObject.h>

namespace anki
{

/// @addtogroup opengl
/// @{

/// Buffer implementation
class BufferImpl : public GlObject
{
public:
	U32 m_size = 0; ///< The size of the buffer
	void* m_persistentMapping = nullptr;
	BufferUsageBit m_usage = BufferUsageBit::NONE;
	BufferMapAccessBit m_access = BufferMapAccessBit::NONE;
	GLenum m_target = GL_NONE; ///< A guess
#if ANKI_ASSERTIONS
	Bool m_mapped = false;
#endif

	BufferImpl(GrManager* manager)
		: GlObject(manager)
	{
	}

	~BufferImpl()
	{
		destroyDeferred(glDeleteBuffers);
	}

	void init(PtrSize size, BufferUsageBit usage, BufferMapAccessBit access);

	void bind(GLenum target, U32 binding, PtrSize offset, PtrSize size) const
	{
		ANKI_ASSERT(isCreated());
		ANKI_ASSERT(offset + size <= m_size);
		ANKI_ASSERT(size > 0);
		glBindBufferRange(target, binding, m_glName, offset, size);
	}

	void write(GLuint pbo, U32 pboOffset, U32 offset, U32 size) const
	{
		ANKI_ASSERT(isCreated());
		ANKI_ASSERT(offset + size <= m_size);

		glCopyNamedBufferSubData(pbo, m_glName, pboOffset, offset, size);
	}

	void fill(PtrSize offset, PtrSize size, U32 value)
	{
		ANKI_ASSERT(isCreated());
		ANKI_ASSERT(offset < m_size);
		ANKI_ASSERT((offset % 4) == 0 && "Should be multiple of 4");

		size = (size == MAX_PTR_SIZE) ? (m_size - offset) : size;
		ANKI_ASSERT(offset + size <= m_size);
		ANKI_ASSERT((size % 4) == 0 && "Should be multiple of 4");

		glClearNamedBufferSubData(m_glName, GL_R32UI, offset, size, GL_RED_INTEGER, GL_UNSIGNED_INT, &value);
	}
};
/// @}

} // end namespace anki
