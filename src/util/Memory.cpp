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
void* mallocAligned(PtrSize size, PtrSize alignmentBytes) throw()
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
#else
#	error "Unimplemented"
#endif
}

//==============================================================================
void freeAligned(void* ptr) throw()
{
#if ANKI_POSIX
	::free(ptr);
#else
#	error "Unimplemented"
#endif
}

//==============================================================================
// HeapMemoryPool                                                              =
//==============================================================================

//==============================================================================
/// The hidden implementation of HeapMemoryPool
class HeapMemoryPool::Implementation: public NonCopyable
{
public:
	std::atomic<U32> m_allocationsCount = {0};
};

//==============================================================================
HeapMemoryPool::HeapMemoryPool(I)
{
	m_impl.reset(new Implementation);
}

//==============================================================================
HeapMemoryPool::~HeapMemoryPool()
{}

//==============================================================================
void* HeapMemoryPool::allocate(PtrSize size, PtrSize alignment) throw()
{
	ANKI_ASSERT(m_impl.get() != nullptr);

	++m_impl->m_allocationsCount;
	return mallocAligned(size, alignment);
}

//==============================================================================
Bool HeapMemoryPool::free(void* ptr) throw()
{
	ANKI_ASSERT(m_impl.get() != nullptr);

	--m_impl->m_allocationsCount;
	freeAligned(ptr);
	return true;
}

//==============================================================================
U32 HeapMemoryPool::getAllocationsCount() const
{
	ANKI_ASSERT(m_impl.get() != nullptr);

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
	Implementation(PtrSize size, PtrSize alignmentBytes)
		:	m_alignmentBytes(alignmentBytes), 
			m_memsize(getAlignedRoundUp(alignmentBytes, size))
	{
		ANKI_ASSERT(m_memsize > 0);
		ANKI_ASSERT(m_alignmentBytes > 0);
		m_memory = (U8*)mallocAligned(m_memsize, m_alignmentBytes);

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
			freeAligned(m_memory);
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
	void* allocate(PtrSize size, PtrSize alignment) throw()
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
	Bool free(void* ptr) throw()
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
StackMemoryPool::StackMemoryPool(PtrSize size, PtrSize alignmentBytes)
{
	m_impl.reset(new Implementation(size, alignmentBytes));
}

//==============================================================================
StackMemoryPool::~StackMemoryPool()
{}

//==============================================================================
PtrSize StackMemoryPool::getTotalSize() const
{
	ANKI_ASSERT(m_impl.get() != nullptr);
	return m_impl->getTotalSize();
}

//==============================================================================
PtrSize StackMemoryPool::getAllocatedSize() const
{
	ANKI_ASSERT(m_impl.get() != nullptr);
	return m_impl->getAllocatedSize();
}

//==============================================================================
void* StackMemoryPool::allocate(PtrSize size, PtrSize alignment) throw()
{
	ANKI_ASSERT(m_impl.get() != nullptr);
	return m_impl->allocate(size, alignment);
}

//==============================================================================
Bool StackMemoryPool::free(void* ptr) throw()
{
	ANKI_ASSERT(m_impl.get() != nullptr);
	return m_impl->free(ptr);
}

//==============================================================================
void StackMemoryPool::reset()
{
	ANKI_ASSERT(m_impl.get() != nullptr);
	m_impl->reset();
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
		U32 m_allocationsCount; 

		Chunk(PtrSize size, PtrSize alignmentBytes)
			:	m_pool(size, alignmentBytes),
				m_allocationsCount(0)
		{}
	};

	/// Alignment of allocations
	PtrSize m_alignmentBytes;

	/// A list of chunks
	/// @note Use STL allocator to keep it out of the grid
	Vector<Chunk*, std::allocator<Chunk*>> m_chunks;

	/// Current chunk to allocate from
	Chunk* m_crntChunk = nullptr;

	/// Fast thread locking
	SpinLock m_lock;

	/// Chunk first chunk size
	PtrSize m_initSize;

	/// Chunk max size
	PtrSize m_maxSize;

	/// Chunk allocation method value
	U32 m_step;

	/// Chunk allocation method
	U8 m_method;

	/// Construct
	Implementation(
		PtrSize initialChunkSize,
		PtrSize maxChunkSize,
		ChunkAllocationStepMethod chunkAllocStepMethod, 
		PtrSize chunkAllocStep, 
		PtrSize alignmentBytes)
		:	m_alignmentBytes(alignmentBytes), 
			m_initSize(initialChunkSize),
			m_maxSize(maxChunkSize),
			m_step((U32)chunkAllocStep),
			m_method(chunkAllocStepMethod)
	{
		// Initial size should be > 0
		ANKI_ASSERT(m_initSize > 0);

		// On fixed step should be 0
		if(m_method == FIXED)
		{
			ANKI_ASSERT(m_step == 0);
		}

		// On fixed initial size is the same as the max
		if(m_method == FIXED)
		{
			ANKI_ASSERT(m_initSize == m_maxSize);
		}

		// On add and mul the max size should be greater than initial
		if(m_method == ADD || m_method == MULTIPLY)
		{
			ANKI_ASSERT(m_initSize < m_maxSize);
		}
	}

	/// Destroy
	~Implementation()
	{
		for(Chunk* ch : m_chunks)
		{
			ANKI_ASSERT(ch);
			delete ch;
		}
	}

	/// Create a new chunk
	Chunk* createNewChunk(PtrSize size) throw()
	{
		//
		// Calculate preferred size
		//
		
		// Get the size of the next chunk
		PtrSize crntMaxSize;
		if(m_method == FIXED)
		{
			crntMaxSize = m_initSize;
		}
		else
		{
			// Get the size of the previous max chunk
			if(m_chunks.size() > 0)
			{
				// Get the size of previous
				crntMaxSize = m_chunks.back()->m_pool.getTotalSize();

				// Increase it
				if(m_method == MULTIPLY)
				{
					crntMaxSize *= m_step;
				}
				else
				{
					ANKI_ASSERT(m_method == ADD);
					crntMaxSize += m_step;
				}
			}
			else
			{
				// No chunks. Choose initial size

				ANKI_ASSERT(m_crntChunk == nullptr);
				crntMaxSize = m_initSize;
			}

			ANKI_ASSERT(crntMaxSize > 0);

			// Fix the size
			crntMaxSize = std::min(crntMaxSize, (PtrSize)m_maxSize);
		}

		size = std::max(crntMaxSize, size) 
			+ sizeof(StackMemoryPool::Implementation::MemoryBlockHeader)
			+ m_alignmentBytes;

		ANKI_ASSERT(size <= m_maxSize && "To big chunk");

		//
		// Create the chunk
		//
		Chunk* chunk = new Chunk(size, m_alignmentBytes);

		if(chunk)
		{
			m_chunks.push_back(chunk);
		}
		else
		{
			throw ANKI_EXCEPTION("Chunk creation failed");
		}
		
		return chunk;
	}

	/// Allocate from chunk
	void* allocateFromChunk(Chunk* ch, PtrSize size, PtrSize alignment) throw()
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
	void* allocate(PtrSize size, PtrSize alignment) throw()
	{
		Chunk* ch;
		void* mem = nullptr;

		m_lock.lock();

		// Get chunk
		ch = m_crntChunk;

		// Create new chunk if needed
		if(ch == nullptr 
			|| (mem = allocateFromChunk(ch, size, alignment)) == nullptr)
		{
			// Create new chunk
			m_crntChunk = createNewChunk(size);
			ch = m_crntChunk;

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
	Bool free(void* ptr) throw()
	{
		m_lock.lock();

		// Get the chunk that ptr belongs to
		decltype(m_chunks)::iterator it;
		for(it = m_chunks.begin(); it != m_chunks.end(); it++)
		{
			Chunk* ch = *it;
			ANKI_ASSERT(ch);

			const U8* from = (const U8*)ch->m_pool.getBaseAddress();
			const U8* to = from + ch->m_pool.getTotalSize();
			const U8* cptr = (const U8*)ptr;
			if(cptr >= from && cptr < to)
			{
				break;
			}
		}

		ANKI_ASSERT(it != m_chunks.end() 
			&& "Not initialized or ptr is incorrect");

		// Decrease the deallocation refcount and if it's zero delete the chunk
		ANKI_ASSERT((*it)->m_allocationsCount > 0);
		if(--(*it)->m_allocationsCount == 0)
		{
			// Chunk is empty. Delete it

			Chunk* ch = *it;
			ANKI_ASSERT(ch);
			
			delete ch;
			m_chunks.erase(it);

			// Change the crntChunk
			ANKI_ASSERT(m_crntChunk);
			if(ch == m_crntChunk)
			{
				// If there are chunks chose the last. It's probably full but
				// let the allocate() create a new one
				if(m_chunks.size() > 0)
				{
					m_crntChunk = m_chunks.back();
					ANKI_ASSERT(m_crntChunk);
				}
				else
				{
					m_crntChunk = nullptr;
				}
			}
		}

		m_lock.unlock();

		return true;
	}

	PtrSize getAllocatedSize() const
	{
		PtrSize sum = 0;
		for(Chunk* ch : m_chunks)
		{
			ANKI_ASSERT(ch);
			sum += ch->m_pool.getAllocatedSize();
		}

		return sum;
	}
};

//==============================================================================
ChainMemoryPool::ChainMemoryPool(
	PtrSize initialChunkSize,
	PtrSize maxChunkSize,
	ChunkAllocationStepMethod chunkAllocStepMethod, 
	PtrSize chunkAllocStep, 
	PtrSize alignmentBytes)
{
	m_impl.reset(new Implementation(
		initialChunkSize, maxChunkSize, chunkAllocStepMethod, chunkAllocStep,
		alignmentBytes));
}

//==============================================================================
ChainMemoryPool::~ChainMemoryPool()
{}

//==============================================================================
void* ChainMemoryPool::allocate(PtrSize size, PtrSize alignment) throw()
{
	return m_impl->allocate(size, alignment);
}

//==============================================================================
Bool ChainMemoryPool::free(void* ptr) throw()
{
	return m_impl->free(ptr);
}

//==============================================================================
PtrSize ChainMemoryPool::getChunksCount() const
{
	return m_impl->m_chunks.size();
}

//==============================================================================
PtrSize ChainMemoryPool::getAllocatedSize() const
{
	return m_impl->getAllocatedSize();
}

} // end namespace anki
