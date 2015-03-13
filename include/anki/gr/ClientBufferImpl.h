// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_CLIENT_BUFFER_IMPL_H
#define ANKI_GR_CLIENT_BUFFER_IMPL_H

#include "anki/gr/GlCommon.h"
#include <memory>

namespace anki {

/// @addtogroup opengl_private
/// @{

/// A client buffer used to pass data to and from the server
class ClientBufferImpl: public NonCopyable
{
public:
	/// Default constructor
	ClientBufferImpl()
	{}

	~ClientBufferImpl()
	{
		destroy();
	}

	/// Create the buffer and allocate memory for the chain's allocator
	ANKI_USE_RESULT Error create(
		const GlCommandBufferAllocator<U8>& chainAlloc, PtrSize size)
	{
		ANKI_ASSERT(size > 0);

		m_alloc = chainAlloc;
		m_ptr = m_alloc.allocate(size);
		m_size = size;
		m_preallocated = false;

		return ErrorCode::NONE;
	}

	/// Create the buffer using preallocated memory
	ANKI_USE_RESULT Error create(void* preallocatedMem, PtrSize size)
	{
		ANKI_ASSERT(size > 0);
		m_ptr = preallocatedMem;
		m_size = size;
		m_preallocated = true;

		return ErrorCode::NONE;
	}

	/// Return the base address
	void* getBaseAddress()
	{
		ANKI_ASSERT(m_size > 0 && m_ptr != 0);
		return m_ptr;
	}

	/// Get size of buffer
	PtrSize getSize() const 
	{
		ANKI_ASSERT(m_size > 0);
		return m_size;
	}

	/// Return the allocator
	GlCommandBufferAllocator<U8> getAllocator() const
	{
		ANKI_ASSERT(!m_preallocated);
		return m_alloc;
	}

private:
	void* m_ptr = nullptr;
	U32 m_size = 0;
	GlCommandBufferAllocator<U8> m_alloc;
	Bool8 m_preallocated;

	void destroy()
	{
		if(!m_preallocated && m_ptr)
		{
			ANKI_ASSERT(m_size > 0);
			m_alloc.deallocate(m_ptr, m_size);

			m_size = 0;
			m_ptr = nullptr;
		}
	}
};
/// @}

} // end namespace anki

#endif

