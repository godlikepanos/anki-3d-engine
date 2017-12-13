// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Buffer.h>
#include <anki/gr/gl/GlObject.h>

namespace anki
{

/// @addtogroup opengl
/// @{

/// Buffer implementation
class BufferImpl final : public Buffer, public GlObject
{
public:
	BufferImpl(GrManager* manager)
		: Buffer(manager)
	{
	}

	~BufferImpl()
	{
		destroyDeferred(getManager(), glDeleteBuffers);
	}

	void init(PtrSize size, BufferUsageBit usage, BufferMapAccessBit access);

	void bind(GLenum target, U32 binding, PtrSize offset, PtrSize range) const
	{
		ANKI_ASSERT(isCreated());

		range = (range == MAX_PTR_SIZE) ? (m_size - offset) : range;
		ANKI_ASSERT(range > 0);
		ANKI_ASSERT(offset + range <= m_size);

		glBindBufferRange(target, binding, m_glName, offset, range);
	}

	void bind(GLenum target, U32 binding, PtrSize offset) const
	{
		bind(target, binding, offset, MAX_PTR_SIZE);
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

	void* map(PtrSize offset, PtrSize range, BufferMapAccessBit access)
	{
		// Wait for its creation
		if(serializeRenderingThread(getManager()))
		{
			return nullptr;
		}

		// Sanity checks
		ANKI_ASSERT(offset + range <= m_size);
		ANKI_ASSERT(m_persistentMapping);

		U8* ptr = static_cast<U8*>(m_persistentMapping);
		ptr += offset;

#if ANKI_EXTRA_CHECKS
		ANKI_ASSERT(!m_mapped);
		m_mapped = true;
#endif
		return static_cast<void*>(ptr);
	}

	void unmap()
	{
#if ANKI_EXTRA_CHECKS
		ANKI_ASSERT(m_mapped);
		m_mapped = false;
#endif
	}

private:
	void* m_persistentMapping = nullptr;
	GLenum m_target = GL_NONE; ///< A guess
#if ANKI_EXTRA_CHECKS
	Bool m_mapped = false;
#endif
};
/// @}

} // end namespace anki
