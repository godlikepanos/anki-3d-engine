// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
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

#define ANKI_OOM_ACTION() ANKI_LOGF("Out of memory")

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
// BaseMemoryPool                                                              =
//==============================================================================

//==============================================================================
BaseMemoryPool::~BaseMemoryPool()
{
	ANKI_ASSERT(m_refcount.load() == 0 && "Refcount should be zero");
}

//==============================================================================
Bool BaseMemoryPool::isCreated() const
{
	return m_allocCb != nullptr;
}

//==============================================================================
void* BaseMemoryPool::allocate(PtrSize size, PtrSize alignmentBytes)
{
	void* out = nullptr;
	switch(m_type)
	{
	case Type::HEAP:
		out = static_cast<HeapMemoryPool*>(this)->allocate(
			size, alignmentBytes);
		break;
	case Type::STACK:
		out = static_cast<StackMemoryPool*>(this)->allocate(
			size, alignmentBytes);
		break;
	case Type::CHAIN:
		out = static_cast<ChainMemoryPool*>(this)->allocate(
			size, alignmentBytes);
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

//==============================================================================
void BaseMemoryPool::free(void* ptr)
{
	switch(m_type)
	{
	case Type::HEAP:
		static_cast<HeapMemoryPool*>(this)->free(ptr);
		break;
	case Type::STACK:
		static_cast<StackMemoryPool*>(this)->free(ptr);
		break;
	case Type::CHAIN:
		static_cast<ChainMemoryPool*>(this)->free(ptr);
		break;
	default:
		ANKI_ASSERT(0);
	}
}

//==============================================================================
// HeapMemoryPool                                                              =
//==============================================================================

//==============================================================================
HeapMemoryPool::HeapMemoryPool()
:	BaseMemoryPool(Type::HEAP)
{}

//==============================================================================
HeapMemoryPool::~HeapMemoryPool()
{
	if(m_allocationsCount.load() != 0)
	{
		ANKI_LOGW("Memory pool destroyed before all memory being released");
	}
}

//==============================================================================
void HeapMemoryPool::create(
	AllocAlignedCallback allocCb, void* allocCbUserData)
{
	ANKI_ASSERT(!isCreated());
	ANKI_ASSERT(m_allocCb == nullptr);
	ANKI_ASSERT(allocCb != nullptr);

	m_allocCb = allocCb;
	m_allocCbUserData = allocCbUserData;
#if ANKI_MEM_SIGNATURES
	m_signature = computeSignature(this);
	m_headerSize = getAlignedRoundUp(MAX_ALIGNMENT, sizeof(Signature));
#endif
}

//==============================================================================
void* HeapMemoryPool::allocate(PtrSize size, PtrSize alignment)
{
	ANKI_ASSERT(isCreated());
#if ANKI_MEM_SIGNATURES
	ANKI_ASSERT(alignment <= MAX_ALIGNMENT && "Wrong assumption");
	size += m_headerSize;
#endif

	void* mem = m_allocCb(m_allocCbUserData, nullptr, size, alignment);

	if(mem != nullptr)
	{
		m_allocationsCount.fetchAdd(1);

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
		ANKI_OOM_ACTION();
	}

	return mem;
}

//==============================================================================
void HeapMemoryPool::free(void* ptr)
{
	ANKI_ASSERT(isCreated());

#if ANKI_MEM_SIGNATURES
	U8* memU8 = static_cast<U8*>(ptr);
	memU8 -= m_headerSize;
	if(memcmp(memU8, &m_signature, sizeof(m_signature)) != 0)
	{
		ANKI_LOGE("Signature missmatch on free");
	}

	ptr = static_cast<void*>(memU8);
#endif
	m_allocationsCount.fetchSub(1);
	m_allocCb(m_allocCbUserData, ptr, 0, 0);
}

//==============================================================================
// StackMemoryPool                                                             =
//==============================================================================

//==============================================================================
StackMemoryPool::StackMemoryPool()
:	BaseMemoryPool(Type::STACK)
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
void StackMemoryPool::create(
	AllocAlignedCallback allocCb, void* allocCbUserData,
	PtrSize size, Bool ignoreDeallocationErrors, PtrSize alignmentBytes)
{
	ANKI_ASSERT(!isCreated());
	ANKI_ASSERT(allocCb);
	ANKI_ASSERT(size > 0);
	ANKI_ASSERT(alignmentBytes > 0);

	m_allocCb = allocCb;
	m_allocCbUserData = allocCbUserData;
	m_alignmentBytes = alignmentBytes;
	m_memsize = getAlignedRoundUp(alignmentBytes, size);
	m_ignoreDeallocationErrors = ignoreDeallocationErrors;

	m_memory = static_cast<U8*>(m_allocCb(
		m_allocCbUserData, nullptr, m_memsize, m_alignmentBytes));

	if(m_memory != nullptr)
	{
#if ANKI_DEBUG
		// Invalidate the memory
		memset(m_memory, 0xCC, m_memsize);
#endif

		// Align allocated memory
		m_top.store(m_memory);

		// Calc header size
		m_headerSize = 
			getAlignedRoundUp(m_alignmentBytes, sizeof(MemoryBlockHeader));
	}
	else
	{
		ANKI_OOM_ACTION();
	}
}

//==============================================================================
void* StackMemoryPool::allocate(PtrSize size, PtrSize alignment)
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(alignment <= m_alignmentBytes);
	(void)alignment;

	size = getAlignedRoundUp(m_alignmentBytes, size + m_headerSize);

	ANKI_ASSERT(size < MAX_U32 && "Too big allocation");

	U8* out = m_top.fetchAdd(size);

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
		m_allocationsCount.fetchAdd(1);
	}
	else
	{
		ANKI_OOM_ACTION();
		out = nullptr;
	}

	return out;
}

//==============================================================================
void StackMemoryPool::free(void* ptr)
{
	ANKI_ASSERT(isCreated());

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
	Bool exchange = m_top.compareExchange(expected, desired);

	// Decrease count
	m_allocationsCount.fetchSub(1);

	// Error if needed
	if(!m_ignoreDeallocationErrors && !exchange)
	{
		ANKI_LOGW("Not top of stack. Deallocation failed. Silently ignoring");
	}
}

//==============================================================================
void StackMemoryPool::reset()
{
	ANKI_ASSERT(isCreated());

	// memory is nullptr if moved
	ANKI_ASSERT(m_memory != nullptr);

#if ANKI_DEBUG
	// Invalidate the memory
	memset(m_memory, 0xCC, m_memsize);
#endif

	m_top.store(m_memory);
	m_allocationsCount.store(0);
}

//==============================================================================
StackMemoryPool::Snapshot StackMemoryPool::getShapshot() const
{
	ANKI_ASSERT(isCreated());
	return m_top.load();
}

//==============================================================================
void StackMemoryPool::resetUsingSnapshot(Snapshot s)
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(static_cast<U8*>(s) >= m_memory);
	ANKI_ASSERT(static_cast<U8*>(s) < m_memory + m_memsize);

	m_top.store(static_cast<U8*>(s));
}

//==============================================================================
// ChainMemoryPool                                                             =
//==============================================================================

//==============================================================================
ChainMemoryPool::ChainMemoryPool()
:	BaseMemoryPool(Type::CHAIN)
{}

//==============================================================================
ChainMemoryPool::~ChainMemoryPool()
{
	if(m_allocationsCount.load() != 0)
	{
		ANKI_LOGW("Memory pool destroyed before all memory being released");
	}

	Chunk* ch = m_headChunk;
	while(ch)
	{
		Chunk* next = ch->m_next;
		destroyChunk(ch);
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
void ChainMemoryPool::create(
	AllocAlignedCallback allocCb, 
	void* allocCbUserData,
	PtrSize initialChunkSize,
	PtrSize maxChunkSize,
	ChunkGrowMethod chunkAllocStepMethod, 
	PtrSize chunkAllocStep, 
	PtrSize alignmentBytes)
{
	ANKI_ASSERT(!isCreated());
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
		ANKI_OOM_ACTION();
	}
	construct(m_lock);

	// Initial size should be > 0
	ANKI_ASSERT(m_initSize > 0 && "Wrong arg");

	// On fixed step should be 0
	if(m_method == ChunkGrowMethod::FIXED)
	{
		ANKI_ASSERT(m_step == 0 && "Wrong arg");
	}

	// On fixed initial size is the same as the max
	if(m_method == ChunkGrowMethod::FIXED)
	{
		ANKI_ASSERT(m_initSize == m_maxSize && "Wrong arg");
	}

	// On add and mul the max size should be greater than initial
	if(m_method == ChunkGrowMethod::ADD 
		|| m_method == ChunkGrowMethod::MULTIPLY)
	{
		ANKI_ASSERT(m_initSize < m_maxSize && "Wrong arg");
	}
}

//==============================================================================
void* ChainMemoryPool::allocate(PtrSize size, PtrSize alignment)
{
	ANKI_ASSERT(isCreated());
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
		PtrSize chunkSize = computeNewChunkSize(size);
		ch = createNewChunk(chunkSize);

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

	m_allocationsCount.fetchAdd(1);

	return mem;
}

//==============================================================================
void ChainMemoryPool::free(void* ptr)
{
	ANKI_ASSERT(isCreated());
	if(ptr == nullptr)
	{
		return;
	}

	LockGuard<SpinLock> lock(*m_lock);

	// Find the chunk the ptr belongs to
	Chunk* chunk = m_headChunk;
	Chunk* prevChunk = nullptr;
	while(chunk)
	{
		const U8* from = chunk->m_memory;
		const U8* to = from + chunk->m_memsize;
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
		destroyChunk(chunk);
	}

	m_allocationsCount.fetchSub(1);
}

//==============================================================================
PtrSize ChainMemoryPool::getChunksCount() const
{
	ANKI_ASSERT(isCreated());

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
	ANKI_ASSERT(isCreated());

	PtrSize sum = 0;
	Chunk* ch = m_headChunk;
	while(ch)
	{
		sum += ch->m_top - ch->m_memory;
		ch = ch->m_next;
	}

	return sum;
}

//==============================================================================
PtrSize ChainMemoryPool::computeNewChunkSize(PtrSize size)
{
	ANKI_ASSERT(size <= m_maxSize);

	// Get the size of the next chunk
	PtrSize crntMaxSize;
	if(m_method == ChunkGrowMethod::FIXED)
	{
		crntMaxSize = m_initSize;
	}
	else
	{
		// Get the size of the previous max chunk
		if(m_tailChunk != nullptr)
		{
			// Get the size of previous
			crntMaxSize = m_tailChunk->m_memsize;
			ANKI_ASSERT(crntMaxSize > 0);

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
		crntMaxSize = min(crntMaxSize, m_maxSize);
	}

	size = max(crntMaxSize, size) + m_alignmentBytes;

	return size;
}

//==============================================================================
ChainMemoryPool::Chunk* ChainMemoryPool::createNewChunk(PtrSize size)
{
	ANKI_ASSERT(size > 0);

	Chunk* chunk = reinterpret_cast<Chunk*>(m_allocCb(
		m_allocCbUserData, nullptr, sizeof(Chunk), alignof(Chunk)));

	if(chunk)
	{
		// Construct it
		construct(chunk);

		// Initialize it
		chunk->m_memory = reinterpret_cast<U8*>(
			m_allocCb(m_allocCbUserData, nullptr, size, m_alignmentBytes));
		if(chunk->m_memory)
		{
			chunk->m_memsize = size;
			chunk->m_top = chunk->m_memory;

#if ANKI_DEBUG
			memset(chunk->m_memory, 0xCC, chunk->m_memsize);
#endif

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
			ANKI_OOM_ACTION();
			destruct(chunk);
			m_allocCb(m_allocCbUserData, chunk, 0, 0);
			chunk = nullptr;
		}
	}
	else
	{
		ANKI_OOM_ACTION();
	}
	
	return chunk;
}

//==============================================================================
void* ChainMemoryPool::allocateFromChunk(
	Chunk* ch, PtrSize size, PtrSize alignment)
{
	ANKI_ASSERT(ch);
	ANKI_ASSERT(size <= m_maxSize);
	ANKI_ASSERT(ch->m_top <= ch->m_memory + ch->m_memsize);

	U8* mem = ch->m_top;
	alignRoundUp(m_alignmentBytes, mem);
	U8* newTop = mem + size;

	if(newTop <= ch->m_memory + ch->m_memsize)
	{
		ch->m_top = newTop;
		++ch->m_allocationsCount;
	}
	else
	{
		// Chunk is full. Need a new one
		mem = nullptr;
	}

	return mem;
}

//==============================================================================
void ChainMemoryPool::destroyChunk(Chunk* ch)
{
	ANKI_ASSERT(ch);

	if(ch->m_memory)
	{
#if ANKI_DEBUG
		memset(ch->m_memory, 0xCC, ch->m_memsize);
#endif
		m_allocCb(m_allocCbUserData, ch->m_memory, 0, 0);
	}

#if ANKI_DEBUG
	memset(ch, 0xCC, sizeof(*ch));
#endif
	m_allocCb(m_allocCbUserData, ch, 0, 0);
}

} // end namespace anki
