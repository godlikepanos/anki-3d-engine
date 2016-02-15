// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
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
	BufferAccessBit m_access = BufferAccessBit::NONE;
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

	void init(PtrSize size, BufferUsageBit usage, BufferAccessBit access);

	void bind(GLenum target, U32 binding, PtrSize offset, PtrSize size) const
	{
		ANKI_ASSERT(isCreated());
		ANKI_ASSERT(offset + size <= m_size);
		ANKI_ASSERT(size > 0);
		glBindBufferRange(target, binding, m_glName, offset, size);
	}

	void write(const void* buff, U32 offset, U32 size) const
	{
		ANKI_ASSERT(isCreated());
		ANKI_ASSERT(offset + size <= m_size);
		ANKI_ASSERT((m_access & BufferAccessBit::CLIENT_WRITE)
			!= BufferAccessBit::NONE);
		glBindBuffer(m_target, m_glName);
		glBufferSubData(m_target, offset, size, buff);
	}
};
/// @}

} // end namespace anki
