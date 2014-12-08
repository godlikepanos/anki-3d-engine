// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/Memory.h"
#include "anki/util/Functions.h"
#include "anki/util/Assert.h"
#include "anki/util/NonCopyable.h"
#include "anki/util/Thread.h"
#include "anki/util/Atomic.h"
#include "anki/util/Logger.h"
#include <cstdlib>
#include <cstring>

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

#define ANKI_MEM_SIGNATURES ANKI_DEBUG

#if ANKI_MEM_SIGNATURES
using Signature = U32;

static Signature computeSignature(void* ptr)
{
	ANKI_ASSERT(ptr);
	PtrSize sig64 = reinterpret_cast<PtrSize>(ptr);
	Signature sig = sig64;
	sig ^= 0x5bd1e995;
	sig ^= sig << 24;
	ANKI_ASSERT(sig != 0);
	return sig;
}
#endif

//==============================================================================
// Other                                                                       =
//==============================================================================

//==============================================================================
void* mallocAligned(PtrSize size, PtrSize alignmentBytes)
{
#if ANKI_POSIX 
#	if ANKI_OS != ANKI_OS_ANDROID
	void* out;
	U alignment = getAlignedRoundUp(alignmentBytes, sizeof(void*));
	int err = posix_memalign(&out, alignment, size);

	if(!err)
	{
		// Make sure it's aligned
		ANKI_ASSERT(isAligned(alignmentBytes, out));
	}
	else
	{
		ANKI_LOGE("mallocAligned() failed");
	}

	return out;
#	else
	void* out = memalign(
		getAlignedRoundUp(alignmentBytes, sizeof(void*)), size);

	if(out)
	{
		// Make sure it's aligned
		ANKI_ASSERT(isAligned(alignmentBytes, out));
	}
	else
	{
		ANKI_LOGE("memalign() failed");
	}

	return out;
#	endif
#elif ANKI_OS == ANKI_OS_WINDOWS
	void* out = _aligned_malloc(size, alignmentBytes);

	if(out)
	{
		// Make sure it's aligned
		ANKI_ASSERT(isAligned(alignmentBytes, out));
	}
	else
	{
		ANKI_LOGE("_aligned_malloc() failed");
	}

	return out;
#else
#	error "Unimplemented"
#endif
}

//==============================================================================
void freeAligned(void* ptr)
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
	void* userData, void* ptr, PtrSize size, PtrSize alignment)
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
HeapMemoryPool::HeapMemoryPool()
{}

//==============================================================================
HeapMemoryPool::~HeapMemoryPool()
{
	ANKI_ASSERT(m_refcount == 0 && "Refcount should be zero");

	if(m_allocationsCount != 0)
	{
		ANKI_LOGW("Memory pool destroyed before all memory being released");
	}
}

//==============================================================================
Error HeapMemoryPool::create(
	AllocAlignedCallback allocCb, void* allocCbUserData)
{
	ANKI_ASSERT(m_allocCb == nullptr);
	ANKI_ASSERT(allocCb != nullptr);

	m_refcount = 0;
	m_allocationsCount = 0;
	m_allocCb = allocCb;
	m_allocCbUserData = allocCbUserData;
#if ANKI_MEM_SIGNATURES
	m_signature = computeSignature(this);
	m_headerSize = getAlignedRoundUp(MAX_ALIGNMENT, sizeof(Signature));
#endif

	return ErrorCode::NONE;
}

//==============================================================================
void* HeapMemoryPool::allocate(PtrSize size, PtrSize alignment)
{	
#if ANKI_MEM_SIGNATURES
	ANKI_ASSERT(alignment <= MAX_ALIGNMENT && "Wrong assumption");
	size += m_headerSize;
#endif

	void* mem = m_allocCb(m_allocCbUserData, nullptr, size, alignment);

	if(mem != nullptr)
	{
		++m_allocationsCount;

#if ANKI_MEM_SIGNATURES
		memset(mem, 0, m_headerSize);
		memcpy(mem, &m_signature, sizeof(m_signature));
		U8* memU8 = static_cast<U8*>(mem);
		memU8 += m_headerSize;
		mem = static_cast<void*>(memU8);
#endif
	}
	else
	{
		ANKI_LOGE("Out of memory");
	}

	return mem;
}

//==============================================================================
Bool HeapMemoryPool::free(void* ptr)
{
#if ANKI_MEM_SIGNATURES
	U8* memU8 = static_cast<U8*>(ptr);
	memU8 -= m_headerSize;
	if(memcmp(memU8, &m_signature, sizeof(m_signature)) != 0)
	{
		ANKI_LOGE("Signature missmatch on free");
	}

	ptr = static_cast<void*>(memU8);
#endif
	--m_allocationsCount;
	m_allocCb(m_allocCbUserData, ptr, 0, 0);

	return true;
}

//==============================================================================
// StackMemoryPool                                                             =
//==============================================================================

//==============================================================================
StackMemoryPool::StackMemoryPool()
{}

//==============================================================================
StackMemoryPool::~StackMemoryPool()
{
	if(m_memory != nullptr)
	{
#if ANKI_DEBUG
		// Invalidate the memory
		memset(m_memory, 0xCC, m_memsize);
#endif
		m_allocCb(m_allocCbUserData, m_memory, 0, 0);
	}
}

//==============================================================================
Error StackMemoryPool::create(
	AllocAlignedCallback allocCb, void* allocCbUserData,
	PtrSize size, PtrSize alignmentBytes)
{
	ANKI_ASSERT(allocCb);
	ANKI_ASSERT(size > 0);
	ANKI_ASSERT(alignmentBytes > 0);

	Error error = ErrorCode::NONE;

	m_refcount = 0;
	m_allocCb = allocCb;
	m_allocCbUserData = allocCbUserData;
	m_alignmentBytes = alignmentBytes;
	m_memsize = getAlignedRoundUp(alignmentBytes, size);
	m_allocationsCount = 0;

	m_memory = reinterpret_cast<U8*>(m_allocCb(
		m_allocCbUserData, nullptr, m_memsize, m_alignmentBytes));

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
		ANKI_LOGE("Out of memory");
		error = ErrorCode::OUT_OF_MEMORY;
	}

	return error;
}

//==============================================================================
void* StackMemoryPool::allocate(PtrSize size, PtrSize alignment)
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
		MemoryBlockHeader* header = 
			reinterpret_cast<MemoryBlockHeader*>(out);
		U32 size32 = size;
		memcpy(&header->m_size[0], &size32, sizeof(U32));

		// Set the correct output
		out += m_headerSize;

		// Check alignment
		ANKI_ASSERT(isAligned(m_alignmentBytes, out));

		// Increase count
		++m_allocationsCount;
	}
	else
	{
		// Not always an error
		if(!m_ignoreAllocationErrors)
		{
			ANKI_LOGE("Out of memory");
		}

		out = nullptr;
	}

	return out;
}

//==============================================================================
Bool StackMemoryPool::free(void* ptr)
{
	// ptr shouldn't be null or not aligned. If not aligned it was not 
	// allocated by this class
	ANKI_ASSERT(ptr != nullptr && isAligned(m_alignmentBytes, ptr));

	// memory is nullptr if moved
	ANKI_ASSERT(m_memory != nullptr);

	// Correct the p
	U8* realptr = reinterpret_cast<U8*>(ptr) - m_headerSize;

	// realptr should be inside the pool's preallocated memory
	ANKI_ASSERT(realptr >= m_memory);

	// Get block size
	MemoryBlockHeader* header = reinterpret_cast<MemoryBlockHeader*>(realptr);
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

	// Decrease count
	--m_allocationsCount;

	return exchange;
}

//==============================================================================
void StackMemoryPool::reset()
{
	// memory is nullptr if moved
	ANKI_ASSERT(m_memory != nullptr);

#if ANKI_DEBUG
	// Invalidate the memory
	memset(m_memory, 0xCC, m_memsize);
#endif

	m_top = m_memory;
	m_allocationsCount = 0;
}

//==============================================================================
StackMemoryPool::Snapshot StackMemoryPool::getShapshot() const
{
	ANKI_ASSERT(m_memory != nullptr);
	return m_top.load();
}

//==============================================================================
void StackMemoryPool::resetUsingSnapshot(Snapshot s)
{
	ANKI_ASSERT(m_memory != nullptr);
	ANKI_ASSERT(static_cast<U8*>(s) >= m_memory);
	ANKI_ASSERT(static_cast<U8*>(s) < m_memory + m_memsize);

	m_top.store(static_cast<U8*>(s));
}

//==============================================================================
// ChainMemoryPool                                                             =
//==============================================================================

//==============================================================================
ChainMemoryPool::ChainMemoryPool()
{}

//==============================================================================
ChainMemoryPool::~ChainMemoryPool()
{
	Chunk* ch = m_headChunk;
	while(ch)
	{
		Chunk* next = ch->m_next;

		ch->~Chunk();
		m_allocCb(m_allocCbUserData, ch, 0, 0);

		ch = next;
	}

	if(m_lock)
	{
		ANKI_ASSERT(m_allocCb);
		m_lock->~SpinLock();
		m_allocCb(m_allocCbUserData, m_lock, 0, 0);
	}
}

//==============================================================================
Error ChainMemoryPool::create(
	AllocAlignedCallback allocCb, 
	void* allocCbUserData,
	PtrSize initialChunkSize,
	PtrSize maxChunkSize,
	ChunkGrowMethod chunkAllocStepMethod, 
	PtrSize chunkAllocStep, 
	PtrSize alignmentBytes)
{
	ANKI_ASSERT(m_allocCb == nullptr);

	// Set all values
	m_allocCb = allocCb;
	m_allocCbUserData = allocCbUserData;
	m_alignmentBytes = alignmentBytes;
	m_initSize = initialChunkSize;
	m_maxSize = maxChunkSize;
	m_step = chunkAllocStep;
	m_method = chunkAllocStepMethod;
	
	m_lock = reinterpret_cast<SpinLock*>(m_allocCb(
		m_allocCbUserData, nullptr, sizeof(SpinLock), alignof(SpinLock)));
	if(!m_lock)
	{
		return ErrorCode::OUT_OF_MEMORY;
	}
	construct(m_lock);

	// Initial size should be > 0
	ANKI_ASSERT(m_initSize > 0 && "Wrong arg");

	// On fixed step should be 0
	if(m_method == ChainMemoryPool::ChunkGrowMethod::FIXED)
	{
		ANKI_ASSERT(m_step == 0 && "Wrong arg");
	}

	// On fixed initial size is the same as the max
	if(m_method == ChainMemoryPool::ChunkGrowMethod::FIXED)
	{
		ANKI_ASSERT(m_initSize == m_maxSize && "Wrong arg");
	}

	// On add and mul the max size should be greater than initial
	if(m_method == ChainMemoryPool::ChunkGrowMethod::ADD 
		|| m_method == ChainMemoryPool::ChunkGrowMethod::MULTIPLY)
	{
		ANKI_ASSERT(m_initSize < m_maxSize && "Wrong arg");
	}

	return ErrorCode::NONE;
}

//==============================================================================
void* ChainMemoryPool::allocate(PtrSize size, PtrSize alignment)
{
	ANKI_ASSERT(m_allocCb);
	ANKI_ASSERT(size <= m_maxSize);

	Chunk* ch;
	void* mem = nullptr;

	LockGuard<SpinLock> lock(*m_lock);

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
			return mem;
		}
	}

	if(mem == nullptr)
	{
		mem = allocateFromChunk(ch, size, alignment);
		ANKI_ASSERT(mem != nullptr && "The chunk should have space");
	}

	++m_allocationsCount;

	return mem;
}

//==============================================================================
Bool ChainMemoryPool::free(void* ptr)
{
	ANKI_ASSERT(m_allocCb);
	if(ptr == nullptr)
	{
		return true;
	}

	LockGuard<SpinLock> lock(*m_lock);

	// Get the chunk that ptr belongs to
	Chunk* chunk = m_headChunk;
	Chunk* prevChunk = nullptr;
	while(chunk)
	{
		const U8* from = chunk->m_pool.m_memory;
		const U8* to = from + chunk->m_pool.m_memsize;
		const U8* cptr = reinterpret_cast<const U8*>(ptr);
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

	--m_allocationsCount;

	return true;
}

//==============================================================================
PtrSize ChainMemoryPool::getChunksCount() const
{
	ANKI_ASSERT(m_allocCb);

	PtrSize count = 0;
	Chunk* ch = m_headChunk;
	while(ch)
	{
		++count;
		ch = ch->m_next;
	}

	return count;
}

//==============================================================================
PtrSize ChainMemoryPool::getAllocatedSize() const
{
	ANKI_ASSERT(m_allocCb);

	PtrSize sum = 0;
	Chunk* ch = m_headChunk;
	while(ch)
	{
		sum += ch->m_pool.getAllocatedSize();
		ch = ch->m_next;
	}

	return sum;
}

//==============================================================================
ChainMemoryPool::Chunk* ChainMemoryPool::createNewChunk(PtrSize size)
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
		crntMaxSize = min(crntMaxSize, (PtrSize)m_maxSize);
	}

	size = max(crntMaxSize, size) + 16;

	//
	// Create the chunk
	//
	Chunk* chunk = (Chunk*)m_allocCb(
		m_allocCbUserData, nullptr, sizeof(Chunk), alignof(Chunk));

	if(chunk)
	{
		// Construct it
		construct(chunk);
		Error error = chunk->m_pool.create(
			m_allocCb, m_allocCbUserData, size, m_alignmentBytes);

		if(!error)
		{
			chunk->m_pool.m_ignoreAllocationErrors = true;

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
		else
		{
			destruct(chunk);
			m_allocCb(m_allocCbUserData, chunk, 0, 0);
			chunk = nullptr;
		}
	}
	else
	{
		ANKI_LOGE("Out of memory");
	}
	
	return chunk;
}

//==============================================================================
void* ChainMemoryPool::allocateFromChunk(
	Chunk* ch, PtrSize size, PtrSize alignment)
{
	ANKI_ASSERT(ch);
	ANKI_ASSERT(size <= m_maxSize);
	void* mem = ch->m_pool.allocate(size, alignment);

	if(mem)
	{
		++ch->m_allocationsCount;
	}
	else
	{
		// Chunk is full. Need a new one
	}

	return mem;
}

} // end namespace anki
