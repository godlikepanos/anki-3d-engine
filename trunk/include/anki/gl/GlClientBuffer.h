#ifndef ANKI_GL_GL_CLIENT_BUFFER_H
#define ANKI_GL_GL_CLIENT_BUFFER_H

#include "anki/gl/GlCommon.h"
#include <memory>

namespace anki {

/// @addtogroup opengl_private
/// @{

/// A client buffer used to pass data to and from the server
class GlClientBuffer: public NonCopyable
{
public:
	/// @name Constructors/Destructor
	/// @{

	/// Default constructor
	GlClientBuffer()
	{}

	/// Create the buffer and allocate memory for the chain's allocator
	GlClientBuffer(const GlJobChainAllocator<U8>& chainAlloc, PtrSize size)
		: m_alloc(chainAlloc)
	{
		ANKI_ASSERT(size > 0);
		m_ptr = m_alloc.allocate(size);
		m_size = size;
		m_preallocated = false;
	}

	/// Create the buffer using preallocated memory
	GlClientBuffer(void* preallocatedMem, PtrSize size)
	{
		ANKI_ASSERT(size > 0);
		m_ptr = preallocatedMem;
		m_size = size;
		m_preallocated = true;
	}

	/// Move
	GlClientBuffer(GlClientBuffer&& b)
	{
		*this = std::move(b);
	}

	~GlClientBuffer()
	{
		destroy();
	}
	/// @}

	/// Move
	GlClientBuffer& operator=(GlClientBuffer&& b)
	{
		destroy();

		m_ptr = b.m_ptr;
		m_size = b.m_size;
		b.m_ptr = nullptr;
		b.m_size = 0;
		m_preallocated = b.m_preallocated;

		if(!m_preallocated)
		{
			m_alloc = b.m_alloc;
			b.m_alloc = GlJobChainAllocator<U8>();
		}
		return *this;
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
	GlJobChainAllocator<U8> getAllocator() const
	{
		ANKI_ASSERT(!m_preallocated);
		return m_alloc;
	}

private:
	void* m_ptr = nullptr;
	U32 m_size = 0;
	GlJobChainAllocator<U8> m_alloc;
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

