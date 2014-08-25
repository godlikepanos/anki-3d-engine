// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/Memory.h"
#include "anki/util/Exception.h"
#include "anki/util/Functions.h"
#include "anki/util/Assert.h"
#include "anki/util/NonCopyable.h"
#include "anki/util/Thread.h"
#include "anki/util/Vector.h"
#include <cstdlib>
#include <cstring>

namespace anki {

//==============================================================================
// Other                                                                       =
//==============================================================================

//==============================================================================
void* mallocAligned(PtrSize size, PtrSize alignmentBytes) noexcept
{
#if ANKI_POSIX 
#	if ANKI_OS != ANKI_OS_ANDROID
	void* out;
	int err = posix_memalign(
		&out, getAlignedRoundUp(alignmentBytes, sizeof(void*)), size);

	if(!err)
	{
		// Make sure it's aligned
		ANKI_ASSERT(isAligned(alignmentBytes, out));
		return out;
	}
	else
	{
		ANKI_ASSERT(0 && "mallocAligned() failed");
		return nullptr;
	}
#	else
	void* out = memalign(
		getAlignedRoundUp(alignmentBytes, sizeof(void*)), size);

	if(out)
	{
		// Make sure it's aligned
		ANKI_ASSERT(isAligned(alignmentBytes, out));
		return out;
	}
	else
	{
		ANKI_ASSERT(0 && "mallocAligned() failed");
		return nullptr;
	}
#	endif
#elif ANKI_OS == ANKI_OS_WINDOWS
	void* out = _aligned_malloc(size, alignmentBytes);

	if(out)
	{
		// Make sure it's aligned
		ANKI_ASSERT(isAligned(alignmentBytes, out));
	}

	return out;
#else
#	error "Unimplemented"
#endif
}

//==============================================================================
void freeAligned(void* ptr) noexcept
{
#if ANKI_POSIX
	::free(ptr);
#elif ANKI_OS == ANKI_OS_WINDOWS
	_aligned_free(ptr);
#else
#	error "Unimplemented"
#endif
}

//==============================================================================
void* allocAligned(
	void* userData, void* ptr, PtrSize size, PtrSize alignment) noexcept
{
	(void)userData;
	void* out;

	if(ptr == nullptr)
	{
		// Allocate
		ANKI_ASSERT(size > 0);
		out = mallocAligned(size, alignment);
	}
	else
	{
		// Deallocate
		ANKI_ASSERT(size == 0);
		ANKI_ASSERT(alignment == 0);

		freeAligned(ptr);
		out = nullptr;
	}

	return out;
}

//==============================================================================
// HeapMemoryPool                                                              =
//==============================================================================

//==============================================================================
/// The hidden implementation of HeapMemoryPool
class HeapMemoryPool::Implementation: public NonCopyable
{
public:
	std::atomic<U32> m_refcount;
	std::atomic<U32> m_allocationsCount;
	AllocAlignedCallback m_allocCb;
	void* m_allocCbUserData;
};

//==============================================================================
HeapMemoryPool::HeapMemoryPool(
	AllocAlignedCallback allocCb, void* allocCbUserData)
{
	ANKI_ASSERT(allocCb != nullptr);

	m_impl = static_cast<Implementation*>(allocCb(allocCbUserData, nullptr, 
		sizeof(Implementation), alignof(Implementation)));

	m_impl->m_refcount = 1;
	m_impl->m_allocationsCount = 0;
	m_impl->m_allocCb = allocCb;
	m_impl->m_allocCbUserData = allocCbUserData;
}

//==============================================================================
HeapMemoryPool& HeapMemoryPool::operator=(const HeapMemoryPool& other)
{
	clear();

	if(other.m_impl)
	{
		m_impl = other.m_impl;
		++m_impl->m_refcount;
	}

	return *this;
}

//==============================================================================
void HeapMemoryPool::clear()
{
	if(m_impl)
	{
		U32 refcount = --m_impl->m_refcount;

		if(refcount == 0)
		{
			auto allocCb = m_impl->m_allocCb;
			auto ud = m_impl->m_allocCbUserData;
			ANKI_ASSERT(allocCb);

			allocCb(ud, m_impl, 0, 0);
		}

		m_impl = nullptr;
	}
}

//==============================================================================
void* HeapMemoryPool::allocate(PtrSize size, PtrSize alignment) noexcept
{
	ANKI_ASSERT(m_impl != nullptr);

	++m_impl->m_allocationsCount;
	return m_impl->m_allocCb(
		m_impl->m_allocCbUserData, nullptr, size, alignment);
}

//==============================================================================
Bool HeapMemoryPool::free(void* ptr) noexcept
{
	ANKI_ASSERT(m_impl != nullptr);

	--m_impl->m_allocationsCount;
	m_impl->m_allocCb(m_impl->m_allocCbUserData, ptr, 0, 0);
	return true;
}

//==============================================================================
U32 HeapMemoryPool::getAllocationsCount() const
{
	ANKI_ASSERT(m_impl != nullptr);

	return m_impl->m_allocationsCount.load();
}

//==============================================================================
// StackMemoryPool                                                             =
//==============================================================================

//==============================================================================
/// The hidden implementation of StackMemoryPool
class StackMemoryPool::Implementation: public NonCopyable
{
public:
	/// The header of each allocation
	class MemoryBlockHeader
	{
	public:
		U8 m_size[sizeof(U32)]; ///< It's U8 to allow whatever alignment
	};

	static_assert(alignof(MemoryBlockHeader) == 1, "Alignment error");
	static_assert(sizeof(MemoryBlockHeader) == sizeof(U32), "Size error");

	/// Refcount
	std::atomic<U32> m_refcount = {1};

	/// User allocation function
	AllocAlignedCallback m_allocCb;

	/// User allocation function data
	void* m_allocCbUserData;

	/// Alignment of allocations
	PtrSize m_alignmentBytes;

	/// Aligned size of MemoryBlockHeader
	PtrSize m_headerSize;

	/// Pre-allocated memory chunk
	U8* m_memory = nullptr;

	/// Size of the pre-allocated memory chunk
	PtrSize m_memsize = 0;

	/// Points to the memory and more specifically to the top of the stack
	std::atomic<U8*> m_top = {nullptr};

	// Construct
	Implementation(AllocAlignedCallback allocCb, void* allocCbUserData,
		PtrSize size, PtrSize alignmentBytes)
	:	m_allocCb(allocCb),
		m_allocCbUserData(allocCbUserData),
		m_alignmentBytes(alignmentBytes), 
		m_memsize(getAlignedRoundUp(alignmentBytes, size))
	{
		ANKI_ASSERT(m_allocCb);
		ANKI_ASSERT(m_memsize > 0);
		ANKI_ASSERT(m_alignmentBytes > 0);
		m_memory = (U8*)m_allocCb(
			m_allocCbUserData, nullptr, m_memsize, m_alignmentBytes);

		if(m_memory != nullptr)
		{
#if ANKI_DEBUG
			// Invalidate the memory
			memset(m_memory, 0xCC, m_memsize);
#endif

			// Align allocated memory
			m_top = m_memory;

			// Calc header size
			m_headerSize = 
				getAlignedRoundUp(m_alignmentBytes, sizeof(MemoryBlockHeader));
		}
		else
		{
			throw ANKI_EXCEPTION("Failed to allocate memory");
		}
	}

	// Destroy
	~Implementation()
	{
		if(m_memory != nullptr)
		{
			m_allocCb(m_allocCbUserData, m_memory, 0, 0);
		}
	}

	PtrSize getTotalSize() const
	{
		return m_memsize;
	}

	PtrSize getAllocatedSize() const
	{
		ANKI_ASSERT(m_memory != nullptr);
		return m_top.load() - m_memory;
	}

	const void* getBaseAddress() const
	{
		ANKI_ASSERT(m_memory != nullptr);
		return m_memory;
	}

	/// Allocate
	void* allocate(PtrSize size, PtrSize alignment) noexcept
	{
		ANKI_ASSERT(m_memory != nullptr);
		ANKI_ASSERT(alignment <= m_alignmentBytes);
		(void)alignment;

		size = getAlignedRoundUp(m_alignmentBytes, size + m_headerSize);

		ANKI_ASSERT(size < MAX_U32 && "Too big allocation");

		U8* out = m_top.fetch_add(size);

		if(out + size <= m_memory + m_memsize)
		{
#if ANKI_DEBUG
			// Invalidate the block
			memset(out, 0xCC, size);
#endif

			// Write the block header
			MemoryBlockHeader* header = (MemoryBlockHeader*)out;
			U32 size32 = size;
			memcpy(&header->m_size[0], &size32, sizeof(U32));

			// Set the correct output
			out += m_headerSize;

			// Check alignment
			ANKI_ASSERT(isAligned(m_alignmentBytes, out));
		}
		else
		{
			// Error
			out = nullptr;
		}

		return out;
	}

	/// Free
	Bool free(void* ptr) noexcept
	{
		// ptr shouldn't be null or not aligned. If not aligned it was not 
		// allocated by this class
		ANKI_ASSERT(ptr != nullptr && isAligned(m_alignmentBytes, ptr));

		// memory is nullptr if moved
		ANKI_ASSERT(m_memory != nullptr);

		// Correct the p
		U8* realptr = (U8*)ptr - m_headerSize;

		// realptr should be inside the pool's preallocated memory
		ANKI_ASSERT(realptr >= m_memory);

		// Get block size
		MemoryBlockHeader* header = (MemoryBlockHeader*)realptr;
		U32 size;
		memcpy(&size, &header->m_size[0], sizeof(U32));

		// Check if the size is within limits
		ANKI_ASSERT(realptr + size <= m_memory + m_memsize);

		// Atomic stuff
		U8* expected = realptr + size;
		U8* desired = realptr;

		// if(top == expected) {
		//     top = desired;
		//     exchange = true;
		// } else {
		//     expected = top;
		//     exchange = false;
		// }
		Bool exchange = m_top.compare_exchange_strong(expected, desired);

		return exchange;
	}

	/// Reset
	void reset()
	{
		// memory is nullptr if moved
		ANKI_ASSERT(m_memory != nullptr);

#if ANKI_DEBUG
		// Invalidate the memory
		memset(m_memory, 0xCC, m_memsize);
#endif

		m_top = m_memory;
	}
};

//==============================================================================
StackMemoryPool::StackMemoryPool(
	AllocAlignedCallback alloc, void* allocUserData,
	PtrSize size, PtrSize alignmentBytes)
{
	m_impl = (Implementation*)alloc(allocUserData, nullptr, 
		sizeof(Implementation), alignof(Implementation));
	
	::new((void*)m_impl) Implementation(
		alloc, allocUserData, size, alignmentBytes);
}

//==============================================================================
StackMemoryPool& StackMemoryPool::operator=(const StackMemoryPool& other)
{
	clear();

	if(other.m_impl)
	{
		m_impl = other.m_impl;
		++m_impl->m_refcount;
	}

	return *this;
}

//==============================================================================
void StackMemoryPool::clear()
{
	if(m_impl)
	{
		U32 refcount = --m_impl->m_refcount;

		if(refcount == 0)
		{
			auto allocCb = m_impl->m_allocCb;
			auto ud = m_impl->m_allocCbUserData;
			ANKI_ASSERT(allocCb);

			m_impl->~Implementation();
			allocCb(ud, m_impl, 0, 0);
		}

		m_impl = nullptr;
	}
}

//==============================================================================
PtrSize StackMemoryPool::getTotalSize() const
{
	ANKI_ASSERT(m_impl != nullptr);
	return m_impl->getTotalSize();
}

//==============================================================================
PtrSize StackMemoryPool::getAllocatedSize() const
{
	ANKI_ASSERT(m_impl != nullptr);
	return m_impl->getAllocatedSize();
}

//==============================================================================
void* StackMemoryPool::allocate(PtrSize size, PtrSize alignment) noexcept
{
	ANKI_ASSERT(m_impl != nullptr);
	return m_impl->allocate(size, alignment);
}

//==============================================================================
Bool StackMemoryPool::free(void* ptr) noexcept
{
	ANKI_ASSERT(m_impl != nullptr);
	return m_impl->free(ptr);
}

//==============================================================================
void StackMemoryPool::reset()
{
	ANKI_ASSERT(m_impl != nullptr);
	m_impl->reset();
}

//==============================================================================
U32 StackMemoryPool::getUsersCount() const
{
	ANKI_ASSERT(m_impl != nullptr);
	return m_impl->m_refcount.load();
}

//==============================================================================
StackMemoryPool::Snapshot StackMemoryPool::getShapshot() const
{
	ANKI_ASSERT(m_impl != nullptr);
	return m_impl->m_top.load();
}

//==============================================================================
void StackMemoryPool::resetUsingSnapshot(Snapshot s)
{
	ANKI_ASSERT(m_impl != nullptr);
	ANKI_ASSERT(static_cast<U8*>(s) >= m_impl->m_memory);
	ANKI_ASSERT(static_cast<U8*>(s) < m_impl->m_memory + m_impl->m_memsize);

	m_impl->m_top.store(static_cast<U8*>(s));
}

//==============================================================================
// ChainMemoryPool                                                             =
//==============================================================================

//==============================================================================
/// The hidden implementation of ChainMemoryPool
class ChainMemoryPool::Implementation: public NonCopyable
{
public:
	/// A chunk of memory
	class Chunk
	{
	public:
		StackMemoryPool::Implementation m_pool;

		/// Used to identify if the chunk can be deleted
		U32 m_allocationsCount = 0;

		/// Next chunk in the list
		Chunk* m_next = nullptr;

		Chunk(AllocAlignedCallback alloc, void* allocUserData, 
			PtrSize size, PtrSize alignmentBytes)
		:	m_pool(alloc, allocUserData, size, alignmentBytes)
		{}
	};

	/// Refcount
	std::atomic<U32> m_refcount = {1};

	/// User allocation function
	AllocAlignedCallback m_allocCb;

	/// User allocation function data
	void* m_allocCbUserData;

	/// Alignment of allocations
	PtrSize m_alignmentBytes;

	/// The first chunk
	Chunk* m_headChunk = nullptr;

	/// Current chunk to allocate from
	Chunk* m_tailChunk = nullptr;

	/// Fast thread locking
	SpinLock m_lock;

	/// Chunk first chunk size
	PtrSize m_initSize;

	/// Chunk max size
	PtrSize m_maxSize;

	/// Chunk allocation method value
	U32 m_step;

	/// Chunk allocation method
	ChainMemoryPool::ChunkGrowMethod m_method;

	/// Construct
	Implementation(
		AllocAlignedCallback allocCb, 
		void* allocCbUserData,
		PtrSize initialChunkSize,
		PtrSize maxChunkSize,
		ChainMemoryPool::ChunkGrowMethod chunkAllocStepMethod, 
		PtrSize chunkAllocStep, 
		PtrSize alignmentBytes)
	:	m_allocCb(allocCb),
		m_allocCbUserData(allocCbUserData),
		m_alignmentBytes(alignmentBytes), 
		m_initSize(initialChunkSize),
		m_maxSize(maxChunkSize),
		m_step((U32)chunkAllocStep),
		m_method(chunkAllocStepMethod)
	{
		ANKI_ASSERT(m_allocCb);

		// Initial size should be > 0
		ANKI_ASSERT(m_initSize > 0);

		// On fixed step should be 0
		if(m_method == ChainMemoryPool::ChunkGrowMethod::FIXED)
		{
			ANKI_ASSERT(m_step == 0);
		}

		// On fixed initial size is the same as the max
		if(m_method == ChainMemoryPool::ChunkGrowMethod::FIXED)
		{
			ANKI_ASSERT(m_initSize == m_maxSize);
		}

		// On add and mul the max size should be greater than initial
		if(m_method == ChainMemoryPool::ChunkGrowMethod::ADD 
			|| m_method == ChainMemoryPool::ChunkGrowMethod::MULTIPLY)
		{
			ANKI_ASSERT(m_initSize < m_maxSize);
		}
	}

	/// Destroy
	~Implementation()
	{
		Chunk* ch = m_headChunk;
		while(ch)
		{
			Chunk* next = ch->m_next;

			ch->~Chunk();
			m_allocCb(m_allocCbUserData, ch, 0, 0);

			ch = next;
		}
	}

	/// Create a new chunk
	Chunk* createNewChunk(PtrSize size) noexcept
	{
		//
		// Calculate preferred size
		//
		
		// Get the size of the next chunk
		PtrSize crntMaxSize;
		if(m_method == ChainMemoryPool::ChunkGrowMethod::FIXED)
		{
			crntMaxSize = m_initSize;
		}
		else
		{
			// Get the size of the previous max chunk
			if(m_tailChunk != nullptr)
			{
				// Get the size of previous
				crntMaxSize = m_tailChunk->m_pool.getTotalSize();

				// Increase it
				if(m_method == ChainMemoryPool::ChunkGrowMethod::MULTIPLY)
				{
					crntMaxSize *= m_step;
				}
				else
				{
					ANKI_ASSERT(m_method 
						== ChainMemoryPool::ChunkGrowMethod::ADD);
					crntMaxSize += m_step;
				}
			}
			else
			{
				// No chunks. Choose initial size

				ANKI_ASSERT(m_headChunk == nullptr);
				crntMaxSize = m_initSize;
			}

			ANKI_ASSERT(crntMaxSize > 0);

			// Fix the size
			crntMaxSize = std::min(crntMaxSize, (PtrSize)m_maxSize);
		}

		size = std::max(crntMaxSize, size) + 16;

		//
		// Create the chunk
		//
		Chunk* chunk = (Chunk*)m_allocCb(
			m_allocCbUserData, nullptr, sizeof(Chunk), alignof(Chunk));

		if(chunk)
		{
			// Construct it
			::new((void*)chunk) Chunk(
				m_allocCb, m_allocCbUserData, size, m_alignmentBytes);

			// Register it
			if(m_tailChunk)
			{
				m_tailChunk->m_next = chunk;
				m_tailChunk = chunk;
			}
			else
			{
				ANKI_ASSERT(m_headChunk == nullptr);

				m_headChunk = m_tailChunk = chunk;
			}
		}
		
		return chunk;
	}

	/// Allocate from chunk
	void* allocateFromChunk(Chunk* ch, PtrSize size, PtrSize alignment) noexcept
	{
		ANKI_ASSERT(ch);
		ANKI_ASSERT(size <= m_maxSize);
		void* mem = ch->m_pool.allocate(size, alignment);

		if(mem)
		{
			++ch->m_allocationsCount;
		}

		return mem;
	}

	/// Allocate memory
	void* allocate(PtrSize size, PtrSize alignment) noexcept
	{
		ANKI_ASSERT(size <= m_maxSize);

		Chunk* ch;
		void* mem = nullptr;

		m_lock.lock();

		// Get chunk
		ch = m_tailChunk;

		// Create new chunk if needed
		if(ch == nullptr 
			|| (mem = allocateFromChunk(ch, size, alignment)) == nullptr)
		{
			// Create new chunk
			ch = createNewChunk(size);

			// Chunk creation failed
			if(ch == nullptr)
			{
				m_lock.unlock();
				return mem;
			}
		}

		if(mem == nullptr)
		{
			mem = allocateFromChunk(ch, size, alignment);
			ANKI_ASSERT(mem != nullptr && "The chunk should have space");
		}

		m_lock.unlock();
		return mem;
	}

	/// Free memory
	Bool free(void* ptr) noexcept
	{
		m_lock.lock();

		// Get the chunk that ptr belongs to
		Chunk* chunk = m_headChunk;
		Chunk* prevChunk = nullptr;
		while(chunk)
		{
			const U8* from = (const U8*)chunk->m_pool.getBaseAddress();
			const U8* to = from + chunk->m_pool.getTotalSize();
			const U8* cptr = (const U8*)ptr;
			if(cptr >= from && cptr < to)
			{
				break;
			}

			prevChunk = chunk;
			chunk = chunk->m_next;
		}

		ANKI_ASSERT(chunk != nullptr 
			&& "Not initialized or ptr is incorrect");

		// Decrease the deallocation refcount and if it's zero delete the chunk
		ANKI_ASSERT(chunk->m_allocationsCount > 0);
		if(--chunk->m_allocationsCount == 0)
		{
			// Chunk is empty. Delete it

			if(prevChunk != nullptr)
			{
				ANKI_ASSERT(m_headChunk != chunk);
				prevChunk->m_next = chunk->m_next;
			}

			if(chunk == m_headChunk)
			{
				ANKI_ASSERT(prevChunk == nullptr);
				m_headChunk = chunk->m_next;
			}

			if(chunk == m_tailChunk)
			{
				m_tailChunk = prevChunk;
			}

			// Finaly delete it
			chunk->~Chunk();
			m_allocCb(m_allocCbUserData, chunk, 0, 0);
		}

		m_lock.unlock();

		return true;
	}

	PtrSize getAllocatedSize() const
	{
		PtrSize sum = 0;
		Chunk* ch = m_headChunk;
		while(ch)
		{
			sum += ch->m_pool.getAllocatedSize();
			ch = ch->m_next;
		}

		return sum;
	}

	PtrSize getChunksCount() const
	{
		PtrSize count = 0;
		Chunk* ch = m_headChunk;
		while(ch)
		{
			++count;
			ch = ch->m_next;
		}

		return count;
	}
};

//==============================================================================
ChainMemoryPool::ChainMemoryPool(
	AllocAlignedCallback alloc, 
	void* allocUserData,
	PtrSize initialChunkSize,
	PtrSize maxChunkSize,
	ChunkGrowMethod chunkAllocStepMethod, 
	PtrSize chunkAllocStep, 
	PtrSize alignmentBytes)
{
	m_impl = (Implementation*)alloc(allocUserData, nullptr, 
		sizeof(Implementation), alignof(Implementation));

	::new((void*)m_impl) Implementation(
		alloc, allocUserData,
		initialChunkSize, maxChunkSize, chunkAllocStepMethod, chunkAllocStep,
		alignmentBytes);
}

//==============================================================================
ChainMemoryPool& ChainMemoryPool::operator=(const ChainMemoryPool& other)
{
	clear();

	if(other.m_impl)
	{
		m_impl = other.m_impl;
		++m_impl->m_refcount;
	}

	return *this;
}

//==============================================================================
void ChainMemoryPool::clear()
{
	if(m_impl)
	{
		U32 refcount = --m_impl->m_refcount;

		if(refcount == 0)
		{
			auto allocCb = m_impl->m_allocCb;
			auto ud = m_impl->m_allocCbUserData;
			ANKI_ASSERT(allocCb);

			allocCb(ud, m_impl, 0, 0);
		}

		m_impl = nullptr;
	}
}

//==============================================================================
void* ChainMemoryPool::allocate(PtrSize size, PtrSize alignment) noexcept
{
	ANKI_ASSERT(m_impl != nullptr);
	return m_impl->allocate(size, alignment);
}

//==============================================================================
Bool ChainMemoryPool::free(void* ptr) noexcept
{
	ANKI_ASSERT(m_impl != nullptr);
	return m_impl->free(ptr);
}

//==============================================================================
PtrSize ChainMemoryPool::getChunksCount() const
{
	ANKI_ASSERT(m_impl != nullptr);
	return m_impl->getChunksCount();
}

//==============================================================================
PtrSize ChainMemoryPool::getAllocatedSize() const
{
	ANKI_ASSERT(m_impl != nullptr);
	return m_impl->getAllocatedSize();
}

//==============================================================================
U32 ChainMemoryPool::getUsersCount() const
{
	ANKI_ASSERT(m_impl != nullptr);
	return m_impl->m_refcount.load();
}

} // end namespace anki
