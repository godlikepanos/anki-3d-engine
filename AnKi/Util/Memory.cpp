// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/Memory.h>
#include <AnKi/Util/Functions.h>
#include <AnKi/Util/Assert.h>
#include <AnKi/Util/Thread.h>
#include <AnKi/Util/Atomic.h>
#include <AnKi/Util/Logger.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>

namespace anki {

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
static void invalidateMemory([[maybe_unused]] TPtr ptr, [[maybe_unused]] TSize size)
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
	PtrSize alignment = getAlignedRoundUp(alignmentBytes, sizeof(void*));
	int err = posix_memalign(&out, alignment, size);

	if(ANKI_LIKELY(!err))
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
		ANKI_UTIL_LOGE("_aligned_malloc() failed. Size %zu, alignment %zu", size, alignmentBytes);
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

void* allocAligned([[maybe_unused]] void* userData, void* ptr, PtrSize size, PtrSize alignment)
{
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

BaseMemoryPool::BaseMemoryPool(Type type, AllocAlignedCallback allocCb, void* allocCbUserData, const char* name)
	: m_allocCb(allocCb)
	, m_allocCbUserData(allocCbUserData)
	, m_type(type)
{
	ANKI_ASSERT(allocCb != nullptr);

	I64 len;
	if(name && (len = strlen(name)) > 0)
	{
		m_name = static_cast<char*>(malloc(len + 1));
		memcpy(m_name, name, len + 1);
	}
}

BaseMemoryPool::~BaseMemoryPool()
{
	ANKI_ASSERT(m_refcount.load() == 0 && "Refcount should be zero");
}

HeapMemoryPool::HeapMemoryPool(AllocAlignedCallback allocCb, void* allocCbUserDataconst, const char* name)
	: BaseMemoryPool(Type::HEAP, allocCb, allocCbUserDataconst, name)
{
#if ANKI_MEM_EXTRA_CHECKS
	m_signature = computePoolSignature(this);
#endif
}

HeapMemoryPool::~HeapMemoryPool()
{
	const U32 count = m_allocationCount.load();
	if(count != 0)
	{
		ANKI_UTIL_LOGE("Memory pool destroyed before all memory being released (%u deallocations missed): %s", count,
					   getName());
	}
}

void* HeapMemoryPool::allocate(PtrSize size, PtrSize alignment)
{
	ANKI_ASSERT(size > 0);
#if ANKI_MEM_EXTRA_CHECKS
	ANKI_ASSERT(alignment <= MAX_ALIGNMENT && "Wrong assumption");
	size += ALLOCATION_HEADER_SIZE;
#endif

	void* mem = m_allocCb(m_allocCbUserData, nullptr, size, alignment);

	if(mem != nullptr)
	{
		m_allocationCount.fetchAdd(1);

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
	m_allocationCount.fetchSub(1);
	m_allocCb(m_allocCbUserData, ptr, 0, 0);
}

Error StackMemoryPool::StackAllocatorBuilderInterface::allocateChunk(PtrSize size, Chunk*& out)
{
	ANKI_ASSERT(size > 0);

	const PtrSize fullChunkSize = offsetof(Chunk, m_memoryStart) + size;

	void* mem = m_parent->m_allocCb(m_parent->m_allocCbUserData, nullptr, fullChunkSize, MAX_ALIGNMENT);

	if(ANKI_LIKELY(mem))
	{
		out = static_cast<Chunk*>(mem);
		invalidateMemory(&out->m_memoryStart[0], size);
	}
	else
	{
		ANKI_OOM_ACTION();
		return Error::OUT_OF_MEMORY;
	}

	return Error::NONE;
}

void StackMemoryPool::StackAllocatorBuilderInterface::freeChunk(Chunk* chunk)
{
	ANKI_ASSERT(chunk);
	m_parent->m_allocCb(m_parent->m_allocCbUserData, chunk, 0, 0);
}

void StackMemoryPool::StackAllocatorBuilderInterface::recycleChunk(Chunk& chunk)
{
	ANKI_ASSERT(chunk.m_chunkSize > 0);
	invalidateMemory(&chunk.m_memoryStart[0], chunk.m_chunkSize);
}

StackMemoryPool::StackMemoryPool(AllocAlignedCallback allocCb, void* allocCbUserData, PtrSize initialChunkSize,
								 F64 nextChunkScale, PtrSize nextChunkBias, Bool ignoreDeallocationErrors,
								 U32 alignmentBytes, const char* name)
	: BaseMemoryPool(Type::STACK, allocCb, allocCbUserData, name)
{
	ANKI_ASSERT(initialChunkSize > 0);
	ANKI_ASSERT(nextChunkScale >= 1.0);
	ANKI_ASSERT(alignmentBytes > 0 && alignmentBytes <= MAX_ALIGNMENT);

	m_builder.getInterface().m_parent = this;
	m_builder.getInterface().m_alignmentBytes = alignmentBytes;
	m_builder.getInterface().m_ignoreDeallocationErrors = ignoreDeallocationErrors;
	m_builder.getInterface().m_initialChunkSize = initialChunkSize;
	m_builder.getInterface().m_nextChunkScale = nextChunkScale;
	m_builder.getInterface().m_nextChunkBias = nextChunkBias;
}

StackMemoryPool::~StackMemoryPool()
{
}

void* StackMemoryPool::allocate(PtrSize size, PtrSize alignment)
{
	ANKI_ASSERT(size > 0);

	Chunk* chunk;
	PtrSize offset;
	if(m_builder.allocate(size, alignment, chunk, offset))
	{
		return nullptr;
	}

	m_allocationCount.fetchAdd(1);
	const PtrSize address = ptrToNumber(&chunk->m_memoryStart[0]) + offset;
	return numberToPtr<void*>(address);
}

void StackMemoryPool::free(void* ptr)
{
	if(ANKI_UNLIKELY(ptr == nullptr))
	{
		return;
	}

	[[maybe_unused]] const U32 count = m_allocationCount.fetchSub(1);
	ANKI_ASSERT(count > 0);
	m_builder.free();
}

void StackMemoryPool::reset()
{
	m_builder.reset();
	m_allocationCount.store(0);
}

ChainMemoryPool::ChainMemoryPool(AllocAlignedCallback allocCb, void* allocCbUserData, PtrSize initialChunkSize,
								 F32 nextChunkScale, PtrSize nextChunkBias, PtrSize alignmentBytes, const char* name)
	: BaseMemoryPool(Type::CHAIN, allocCb, allocCbUserData, name)
{
	ANKI_ASSERT(initialChunkSize > 0);
	ANKI_ASSERT(nextChunkScale >= 1.0);
	ANKI_ASSERT(alignmentBytes > 0);

	// Set all values
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

ChainMemoryPool::~ChainMemoryPool()
{
	if(m_allocationCount.load() != 0)
	{
		ANKI_UTIL_LOGE("Memory pool destroyed before all memory being released");
	}

	Chunk* ch = m_headChunk;
	while(ch)
	{
		Chunk* next = ch->m_next;
		destroyChunk(ch);
		ch = next;
	}
}

void* ChainMemoryPool::allocate(PtrSize size, PtrSize alignment)
{
	ANKI_ASSERT(size > 0);

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

	m_allocationCount.fetchAdd(1);

	return mem;
}

void ChainMemoryPool::free(void* ptr)
{
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
	ANKI_ASSERT(chunk->m_allocationCount > 0);
	if(--chunk->m_allocationCount == 0)
	{
		// Chunk is empty. Delete it
		destroyChunk(chunk);
	}

	m_allocationCount.fetchSub(1);
}

PtrSize ChainMemoryPool::getChunksCount() const
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

PtrSize ChainMemoryPool::getAllocatedSize() const
{
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

void* ChainMemoryPool::allocateFromChunk(Chunk* ch, PtrSize size, [[maybe_unused]] PtrSize alignment)
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
		++ch->m_allocationCount;
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
