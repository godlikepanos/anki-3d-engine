// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/Memory.h>
#include <AnKi/Util/Functions.h>
#include <AnKi/Util/Assert.h>
#include <AnKi/Util/NonCopyable.h>
#include <AnKi/Util/Thread.h>
#include <AnKi/Util/Atomic.h>
#include <AnKi/Util/Logger.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>

namespace anki
{

#if ANKI_MEM_EXTRA_CHECKS
static PoolSignature computePoolSignature(void* ptr)
{
	ANKI_ASSERT(ptr);
	PtrSize sig64 = ptrToNumber(ptr);
	PoolSignature sig = PoolSignature(sig64);
	sig ^= 0x5bd1e995;
	sig ^= sig << 24;
	ANKI_ASSERT(sig != 0);
	return sig;
}

class AllocationHeader
{
public:
	PtrSize m_allocationSize;
	PoolSignature m_signature;
};

constexpr U32 MAX_ALIGNMENT = 64;
constexpr U32 ALLOCATION_HEADER_SIZE = getAlignedRoundUp(MAX_ALIGNMENT, sizeof(AllocationHeader));
#endif

#define ANKI_CREATION_OOM_ACTION() ANKI_UTIL_LOGF("Out of memory")
#define ANKI_OOM_ACTION() ANKI_UTIL_LOGE("Out of memory. Expect segfault")

template<typename TPtr, typename TSize>
static void invalidateMemory(TPtr ptr, TSize size)
{
#if ANKI_MEM_EXTRA_CHECKS
	memset(static_cast<void*>(ptr), 0xCC, size);
#endif
}

void* mallocAligned(PtrSize size, PtrSize alignmentBytes)
{
	ANKI_ASSERT(size > 0);
	ANKI_ASSERT(alignmentBytes > 0);

#if ANKI_POSIX
#	if !ANKI_OS_ANDROID
	void* out = nullptr;
	U alignment = getAlignedRoundUp(alignmentBytes, sizeof(void*));
	int err = posix_memalign(&out, alignment, size);

	if(!err)
	{
		ANKI_ASSERT(out != nullptr);
		// Make sure it's aligned
		ANKI_ASSERT(isAligned(alignmentBytes, out));
	}
	else
	{
		ANKI_UTIL_LOGE("mallocAligned() failed");
	}

	return out;
#	else
	void* out = memalign(getAlignedRoundUp(alignmentBytes, sizeof(void*)), size);

	if(out)
	{
		// Make sure it's aligned
		ANKI_ASSERT(isAligned(alignmentBytes, out));
	}
	else
	{
		ANKI_UTIL_LOGE("memalign() failed");
	}

	return out;
#	endif
#elif ANKI_OS_WINDOWS
	void* out = _aligned_malloc(size, alignmentBytes);

	if(out)
	{
		// Make sure it's aligned
		ANKI_ASSERT(isAligned(alignmentBytes, out));
	}
	else
	{
		ANKI_UTIL_LOGE("_aligned_malloc() failed");
	}

	return out;
#else
#	error "Unimplemented"
#endif
}

void freeAligned(void* ptr)
{
#if ANKI_POSIX
	::free(ptr);
#elif ANKI_OS_WINDOWS
	_aligned_free(ptr);
#else
#	error "Unimplemented"
#endif
}

void* allocAligned(void* userData, void* ptr, PtrSize size, PtrSize alignment)
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

BaseMemoryPool::~BaseMemoryPool()
{
	ANKI_ASSERT(m_refcount.load() == 0 && "Refcount should be zero");
}

Bool BaseMemoryPool::isInitialized() const
{
	return m_allocCb != nullptr;
}

HeapMemoryPool::HeapMemoryPool()
	: BaseMemoryPool(Type::HEAP)
{
}

HeapMemoryPool::~HeapMemoryPool()
{
	const U32 count = m_allocationsCount.load();
	if(count != 0)
	{
		ANKI_UTIL_LOGW("Memory pool destroyed before all memory being released "
					   "(%u deallocations missed)",
					   count);
	}
}

void HeapMemoryPool::init(AllocAlignedCallback allocCb, void* allocCbUserData)
{
	ANKI_ASSERT(!isInitialized());
	ANKI_ASSERT(m_allocCb == nullptr);
	ANKI_ASSERT(allocCb != nullptr);

	m_allocCb = allocCb;
	m_allocCbUserData = allocCbUserData;
#if ANKI_MEM_EXTRA_CHECKS
	m_signature = computePoolSignature(this);
#endif
}

void* HeapMemoryPool::allocate(PtrSize size, PtrSize alignment)
{
	ANKI_ASSERT(isInitialized());
#if ANKI_MEM_EXTRA_CHECKS
	ANKI_ASSERT(alignment <= MAX_ALIGNMENT && "Wrong assumption");
	size += ALLOCATION_HEADER_SIZE;
#endif

	void* mem = m_allocCb(m_allocCbUserData, nullptr, size, alignment);

	if(mem != nullptr)
	{
		m_allocationsCount.fetchAdd(1);

#if ANKI_MEM_EXTRA_CHECKS
		memset(mem, 0, ALLOCATION_HEADER_SIZE);
		AllocationHeader& header = *static_cast<AllocationHeader*>(mem);
		header.m_signature = m_signature;
		header.m_allocationSize = size;

		mem = static_cast<void*>(static_cast<U8*>(mem) + ALLOCATION_HEADER_SIZE);
#endif
	}
	else
	{
		ANKI_OOM_ACTION();
	}

	return mem;
}

void HeapMemoryPool::free(void* ptr)
{
	ANKI_ASSERT(isInitialized());

	if(ANKI_UNLIKELY(ptr == nullptr))
	{
		return;
	}

#if ANKI_MEM_EXTRA_CHECKS
	U8* memU8 = static_cast<U8*>(ptr) - ALLOCATION_HEADER_SIZE;
	AllocationHeader& header = *reinterpret_cast<AllocationHeader*>(memU8);
	if(header.m_signature != m_signature)
	{
		ANKI_UTIL_LOGE("Signature missmatch on free");
	}

	ptr = static_cast<void*>(memU8);
	invalidateMemory(ptr, header.m_allocationSize);
#endif
	m_allocationsCount.fetchSub(1);
	m_allocCb(m_allocCbUserData, ptr, 0, 0);
}

StackMemoryPool::StackMemoryPool()
	: BaseMemoryPool(Type::STACK)
{
}

StackMemoryPool::~StackMemoryPool()
{
	// Iterate all until you find an unused
	for(Chunk& ch : m_chunks)
	{
		if(ch.m_baseMem != nullptr)
		{
			ch.check();

			invalidateMemory(ch.m_baseMem, ch.m_size);
			m_allocCb(m_allocCbUserData, ch.m_baseMem, 0, 0);
		}
		else
		{
			break;
		}
	}

	// Do some error checks
	const U32 allocCount = m_allocationsCount.load();
	if(!m_ignoreDeallocationErrors && allocCount != 0)
	{
		ANKI_UTIL_LOGW("Forgot to deallocate");
	}
}

void StackMemoryPool::init(AllocAlignedCallback allocCb, void* allocCbUserData, PtrSize initialChunkSize,
						   F32 nextChunkScale, PtrSize nextChunkBias, Bool ignoreDeallocationErrors,
						   PtrSize alignmentBytes)
{
	ANKI_ASSERT(!isInitialized());
	ANKI_ASSERT(allocCb);
	ANKI_ASSERT(initialChunkSize > 0);
	ANKI_ASSERT(nextChunkScale >= 1.0);
	ANKI_ASSERT(alignmentBytes > 0);

	m_allocCb = allocCb;
	m_allocCbUserData = allocCbUserData;
	m_alignmentBytes = alignmentBytes;
	m_initialChunkSize = initialChunkSize;
	m_nextChunkScale = nextChunkScale;
	m_nextChunkBias = nextChunkBias;
	m_ignoreDeallocationErrors = ignoreDeallocationErrors;
}

void* StackMemoryPool::allocate(PtrSize size, PtrSize alignment)
{
	ANKI_ASSERT(isInitialized());
	ANKI_ASSERT(alignment <= m_alignmentBytes);
	(void)alignment;

	size = getAlignedRoundUp(m_alignmentBytes, size);
	ANKI_ASSERT(size > 0);

	U8* out = nullptr;

	while(true)
	{
		// Try to allocate from the current chunk, if there is one
		Chunk* crntChunk = nullptr;
		const I32 crntChunkIdx = m_crntChunkIdx.load();
		if(crntChunkIdx >= 0)
		{
			crntChunk = &m_chunks[crntChunkIdx];
			crntChunk->check();

			out = crntChunk->m_mem.fetchAdd(size);
			ANKI_ASSERT(out >= crntChunk->m_baseMem);
		}

		if(crntChunk && out + size <= crntChunk->m_baseMem + crntChunk->m_size)
		{
			// All is fine, there is enough space in the chunk

			m_allocationsCount.fetchAdd(1);
			break;
		}
		else
		{
			// Need new chunk

			LockGuard<Mutex> lock(m_lock);

			// Make sure that only one thread will create a new chunk
			const Bool someOtherThreadCreateAChunkWhileIWasHoldingTheLock = m_crntChunkIdx.load() != crntChunkIdx;
			if(someOtherThreadCreateAChunkWhileIWasHoldingTheLock)
			{
				continue;
			}

			// We can create a new chunk

			ANKI_ASSERT(crntChunkIdx >= -1);
			if(U32(crntChunkIdx + 1) >= m_chunks.getSize())
			{
				out = nullptr;
				ANKI_UTIL_LOGE("Number of chunks is not enough");
				ANKI_OOM_ACTION();
				break;
			}

			// Compute the memory of the new chunk. Don't look at the previous chunk
			PtrSize newChunkSize = m_initialChunkSize;
			for(I i = 0; i < crntChunkIdx + 1; ++i)
			{
				newChunkSize = PtrSize(F64(newChunkSize) * m_nextChunkScale) + m_nextChunkBias;
			}

			newChunkSize = max(size, newChunkSize); // Can't have the allocation fail
			alignRoundUp(m_alignmentBytes, newChunkSize); // Always align at the end

			// Point to the next chunk
			Chunk* newChunk = &m_chunks[crntChunkIdx + 1];

			if(newChunk->m_baseMem == nullptr || newChunk->m_size != newChunkSize)
			{
				// Chunk is empty or its memory doesn't match the expected, need to (re)initialize it

				if(newChunk->m_baseMem)
				{
					m_allocCb(m_allocCbUserData, newChunk->m_baseMem, 0, 0);
					m_allocatedMemory -= newChunk->m_size;
				}

				void* mem = m_allocCb(m_allocCbUserData, nullptr, newChunkSize, m_alignmentBytes);

				if(mem != nullptr)
				{
					invalidateMemory(mem, newChunkSize);

					newChunk->m_baseMem = static_cast<U8*>(mem);
					newChunk->m_mem.setNonAtomically(newChunk->m_baseMem);
					newChunk->m_size = newChunkSize;

					m_allocatedMemory += newChunk->m_size;

					const I32 idx = m_crntChunkIdx.fetchAdd(1);
					ANKI_ASSERT(idx == crntChunkIdx);
					(void)idx;
				}
				else
				{
					out = nullptr;
					ANKI_OOM_ACTION();
					break;
				}
			}
			else
			{
				// Will recycle

				newChunk->checkReset();
				invalidateMemory(newChunk->m_baseMem, newChunk->m_size);

				const I32 idx = m_crntChunkIdx.fetchAdd(1);
				ANKI_ASSERT(idx == crntChunkIdx);
				(void)idx;
			}
		}
	}

	return static_cast<void*>(out);
}

void StackMemoryPool::free(void* ptr)
{
	ANKI_ASSERT(isInitialized());

	if(ANKI_UNLIKELY(ptr == nullptr))
	{
		return;
	}

	// ptr shouldn't be null or not aligned. If not aligned it was not allocated by this class
	ANKI_ASSERT(ptr != nullptr && isAligned(m_alignmentBytes, ptr));

	const U32 count = m_allocationsCount.fetchSub(1);
	ANKI_ASSERT(count > 0);
	(void)count;
}

void StackMemoryPool::reset()
{
	ANKI_ASSERT(isInitialized());

	// Iterate all until you find an unused
	for(Chunk& ch : m_chunks)
	{
		if(ch.m_baseMem != nullptr)
		{
			ch.check();
			ch.m_mem.store(ch.m_baseMem);

			invalidateMemory(ch.m_baseMem, ch.m_size);
		}
		else
		{
			break;
		}
	}

	// Set the crnt chunk
	m_crntChunkIdx.setNonAtomically(-1);

	// Reset allocation count and do some error checks
	const U32 allocCount = m_allocationsCount.exchange(0);
	if(!m_ignoreDeallocationErrors && allocCount != 0)
	{
		ANKI_UTIL_LOGW("Forgot to deallocate");
	}
}

ChainMemoryPool::ChainMemoryPool()
	: BaseMemoryPool(Type::CHAIN)
{
}

ChainMemoryPool::~ChainMemoryPool()
{
	if(m_allocationsCount.load() != 0)
	{
		ANKI_UTIL_LOGW("Memory pool destroyed before all memory being released");
	}

	Chunk* ch = m_headChunk;
	while(ch)
	{
		Chunk* next = ch->m_next;
		destroyChunk(ch);
		ch = next;
	}
}

void ChainMemoryPool::init(AllocAlignedCallback allocCb, void* allocCbUserData, PtrSize initialChunkSize,
						   F32 nextChunkScale, PtrSize nextChunkBias, PtrSize alignmentBytes)
{
	ANKI_ASSERT(!isInitialized());
	ANKI_ASSERT(initialChunkSize > 0);
	ANKI_ASSERT(nextChunkScale >= 1.0);
	ANKI_ASSERT(alignmentBytes > 0);

	// Set all values
	m_allocCb = allocCb;
	m_allocCbUserData = allocCbUserData;
	m_alignmentBytes = alignmentBytes;
	m_initSize = initialChunkSize;
	m_scale = nextChunkScale;
	m_bias = nextChunkBias;
	m_headerSize = max<PtrSize>(m_alignmentBytes, sizeof(Chunk*));

	// Initial size should be > 0
	ANKI_ASSERT(m_initSize > 0 && "Wrong arg");

	// On fixed initial size is the same as the max
	if(m_scale == 0.0 && m_bias == 0)
	{
		ANKI_ASSERT(0 && "Wrong arg");
	}
}

void* ChainMemoryPool::allocate(PtrSize size, PtrSize alignment)
{
	ANKI_ASSERT(isInitialized());

	Chunk* ch;
	void* mem = nullptr;

	LockGuard<SpinLock> lock(m_lock);

	// Get chunk
	ch = m_tailChunk;

	// Create new chunk if needed
	if(ch == nullptr || (mem = allocateFromChunk(ch, size, alignment)) == nullptr)
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

void ChainMemoryPool::free(void* ptr)
{
	ANKI_ASSERT(isInitialized());

	if(ANKI_UNLIKELY(ptr == nullptr))
	{
		return;
	}

	// Get the chunk
	U8* mem = static_cast<U8*>(ptr);
	mem -= m_headerSize;
	Chunk* chunk = *reinterpret_cast<Chunk**>(mem);

	ANKI_ASSERT(chunk != nullptr);
	ANKI_ASSERT((mem >= chunk->m_memory && mem < (chunk->m_memory + chunk->m_memsize)) && "Wrong chunk");

	LockGuard<SpinLock> lock(m_lock);

	// Decrease the deallocation refcount and if it's zero delete the chunk
	ANKI_ASSERT(chunk->m_allocationsCount > 0);
	if(--chunk->m_allocationsCount == 0)
	{
		// Chunk is empty. Delete it
		destroyChunk(chunk);
	}

	m_allocationsCount.fetchSub(1);
}

PtrSize ChainMemoryPool::getChunksCount() const
{
	ANKI_ASSERT(isInitialized());

	PtrSize count = 0;
	Chunk* ch = m_headChunk;
	while(ch)
	{
		++count;
		ch = ch->m_next;
	}

	return count;
}

PtrSize ChainMemoryPool::getAllocatedSize() const
{
	ANKI_ASSERT(isInitialized());

	PtrSize sum = 0;
	Chunk* ch = m_headChunk;
	while(ch)
	{
		sum += ch->m_top - ch->m_memory;
		ch = ch->m_next;
	}

	return sum;
}

PtrSize ChainMemoryPool::computeNewChunkSize(PtrSize size) const
{
	size += m_headerSize;

	PtrSize crntMaxSize;
	if(m_tailChunk != nullptr)
	{
		// Get the size of previous
		crntMaxSize = m_tailChunk->m_memsize;

		// Compute new size
		crntMaxSize = PtrSize(F32(crntMaxSize) * m_scale) + m_bias;
	}
	else
	{
		// No chunks. Choose initial size
		ANKI_ASSERT(m_headChunk == nullptr);
		crntMaxSize = m_initSize;
	}

	crntMaxSize = max(crntMaxSize, size);

	ANKI_ASSERT(crntMaxSize > 0);

	return crntMaxSize;
}

ChainMemoryPool::Chunk* ChainMemoryPool::createNewChunk(PtrSize size)
{
	ANKI_ASSERT(size > 0);

	// Allocate memory and chunk in one go
	PtrSize chunkAllocSize = getAlignedRoundUp(m_alignmentBytes, sizeof(Chunk));
	PtrSize memAllocSize = getAlignedRoundUp(m_alignmentBytes, size);
	PtrSize allocationSize = chunkAllocSize + memAllocSize;

	Chunk* chunk = reinterpret_cast<Chunk*>(m_allocCb(m_allocCbUserData, nullptr, allocationSize, m_alignmentBytes));

	if(chunk)
	{
		invalidateMemory(chunk, allocationSize);

		// Construct it
		memset(chunk, 0, sizeof(Chunk));

		// Initialize it
		chunk->m_memory = reinterpret_cast<U8*>(chunk) + chunkAllocSize;

		chunk->m_memsize = memAllocSize;
		chunk->m_top = chunk->m_memory;

		// Register it
		if(m_tailChunk)
		{
			m_tailChunk->m_next = chunk;
			chunk->m_prev = m_tailChunk;
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
	}

	return chunk;
}

void* ChainMemoryPool::allocateFromChunk(Chunk* ch, PtrSize size, PtrSize alignment)
{
	ANKI_ASSERT(ch);
	ANKI_ASSERT(ch->m_top <= ch->m_memory + ch->m_memsize);

	U8* mem = ch->m_top;
	PtrSize memV = ptrToNumber(mem);
	alignRoundUp(m_alignmentBytes, memV);
	mem = numberToPtr<U8*>(memV);
	U8* newTop = mem + m_headerSize + size;

	if(newTop <= ch->m_memory + ch->m_memsize)
	{
		*reinterpret_cast<Chunk**>(mem) = ch;
		mem += m_headerSize;

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

void ChainMemoryPool::destroyChunk(Chunk* ch)
{
	ANKI_ASSERT(ch);

	if(ch == m_tailChunk)
	{
		m_tailChunk = ch->m_prev;
	}

	if(ch == m_headChunk)
	{
		m_headChunk = ch->m_next;
	}

	if(ch->m_prev)
	{
		ANKI_ASSERT(ch->m_prev->m_next == ch);
		ch->m_prev->m_next = ch->m_next;
	}

	if(ch->m_next)
	{
		ANKI_ASSERT(ch->m_next->m_prev == ch);
		ch->m_next->m_prev = ch->m_prev;
	}

	invalidateMemory(ch, getAlignedRoundUp(m_alignmentBytes, sizeof(Chunk)) + ch->m_memsize);
	m_allocCb(m_allocCbUserData, ch, 0, 0);
}

} // end namespace anki
