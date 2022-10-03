// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/MemoryPool.h>
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

constexpr U32 kExtraChecksMaxAlignment = 64;
constexpr U32 kAllocationHeaderSize = getAlignedRoundUp(kExtraChecksMaxAlignment, U32(sizeof(AllocationHeader)));

template<typename TPtr, typename TSize>
static void invalidateMemory([[maybe_unused]] TPtr ptr, [[maybe_unused]] TSize size)
{
	memset(static_cast<void*>(ptr), 0xCC, size);
}
#endif

#define ANKI_OOM_ACTION() ANKI_UTIL_LOGE("Out of memory. Expect segfault")

void* mallocAligned(PtrSize size, PtrSize alignmentBytes)
{
	ANKI_ASSERT(size > 0);
	ANKI_ASSERT(alignmentBytes > 0);

#if ANKI_OS_ANDROID
	void* out = memalign(getAlignedRoundUp(alignmentBytes, sizeof(void*)), size);

	if(out)
	{
		// Make sure it's aligned
		ANKI_ASSERT(isAligned(alignmentBytes, out));
	}
	else
	{
		ANKI_UTIL_LOGE("memalign() failed, Size %zu, alignment %zu", size, alignmentBytes);
	}

	return out;
#elif ANKI_POSIX
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
		ANKI_UTIL_LOGE("posix_memalign() failed, Size %zu, alignment %zu", size, alignmentBytes);
	}

	return out;
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

void BaseMemoryPool::init(AllocAlignedCallback allocCb, void* allocCbUserData, const Char* name)
{
	ANKI_ASSERT(m_allocCb == nullptr && m_name == nullptr);

	ANKI_ASSERT(allocCb != nullptr);
	m_allocCb = allocCb;
	m_allocCbUserData = allocCbUserData;

	PtrSize len;
	if(name && (len = strlen(name)) > 0)
	{
		m_name = static_cast<char*>(m_allocCb(m_allocCbUserData, nullptr, len + 1, 1));
		memcpy(m_name, name, len + 1);
	}
}

void BaseMemoryPool::destroy()
{
	if(m_name != nullptr)
	{
		m_allocCb(m_allocCbUserData, m_name, 0, 0);
		m_name = nullptr;
	}
	m_allocCb = nullptr;
	m_allocationCount.setNonAtomically(0);
}

void HeapMemoryPool::init(AllocAlignedCallback allocCb, void* allocCbUserData, const Char* name)
{
	BaseMemoryPool::init(allocCb, allocCbUserData, name);
#if ANKI_MEM_EXTRA_CHECKS
	m_signature = computePoolSignature(this);
#endif
}

void HeapMemoryPool::destroy()
{
	const U32 count = m_allocationCount.load();
	if(count != 0)
	{
		ANKI_UTIL_LOGE("Memory pool destroyed before all memory being released (%u deallocations missed): %s", count,
					   getName());
	}
	BaseMemoryPool::destroy();
}

void* HeapMemoryPool::allocate(PtrSize size, PtrSize alignment)
{
	ANKI_ASSERT(size > 0);
#if ANKI_MEM_EXTRA_CHECKS
	ANKI_ASSERT(alignment <= kExtraChecksMaxAlignment && "Wrong assumption");
	size += kAllocationHeaderSize;
#endif

	void* mem = m_allocCb(m_allocCbUserData, nullptr, size, alignment);

	if(mem != nullptr)
	{
		m_allocationCount.fetchAdd(1);

#if ANKI_MEM_EXTRA_CHECKS
		memset(mem, 0, kAllocationHeaderSize);
		AllocationHeader& header = *static_cast<AllocationHeader*>(mem);
		header.m_signature = m_signature;
		header.m_allocationSize = size;

		mem = static_cast<void*>(static_cast<U8*>(mem) + kAllocationHeaderSize);
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
	U8* memU8 = static_cast<U8*>(ptr) - kAllocationHeaderSize;
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
	ANKI_ASSERT(m_parent && m_parent->m_allocCb);

	const PtrSize fullChunkSize = offsetof(Chunk, m_memoryStart) + size;

	void* mem = m_parent->m_allocCb(m_parent->m_allocCbUserData, nullptr, fullChunkSize, kMaxAlignment);

	if(ANKI_LIKELY(mem))
	{
		out = static_cast<Chunk*>(mem);
#if ANKI_MEM_EXTRA_CHECKS
		invalidateMemory(&out->m_memoryStart[0], size);
#endif
	}
	else
	{
		ANKI_OOM_ACTION();
		return Error::kOutOfMemory;
	}

	return Error::kNone;
}

void StackMemoryPool::StackAllocatorBuilderInterface::freeChunk(Chunk* chunk)
{
	ANKI_ASSERT(chunk);
	ANKI_ASSERT(m_parent && m_parent->m_allocCb);
	m_parent->m_allocCb(m_parent->m_allocCbUserData, chunk, 0, 0);
}

void StackMemoryPool::StackAllocatorBuilderInterface::recycleChunk(Chunk& chunk)
{
	ANKI_ASSERT(chunk.m_chunkSize > 0);
#if ANKI_MEM_EXTRA_CHECKS
	invalidateMemory(&chunk.m_memoryStart[0], chunk.m_chunkSize);
#endif
}

void StackMemoryPool::init(AllocAlignedCallback allocCb, void* allocCbUserData, PtrSize initialChunkSize,
						   F64 nextChunkScale, PtrSize nextChunkBias, Bool ignoreDeallocationErrors, U32 alignmentBytes,
						   const Char* name)
{
	ANKI_ASSERT(initialChunkSize > 0);
	ANKI_ASSERT(nextChunkScale >= 1.0);
	ANKI_ASSERT(alignmentBytes > 0 && alignmentBytes <= kMaxAlignment);
	BaseMemoryPool::init(allocCb, allocCbUserData, name);

	m_builder.getInterface().m_parent = this;
	m_builder.getInterface().m_alignmentBytes = alignmentBytes;
	m_builder.getInterface().m_ignoreDeallocationErrors = ignoreDeallocationErrors;
	m_builder.getInterface().m_initialChunkSize = initialChunkSize;
	m_builder.getInterface().m_nextChunkScale = nextChunkScale;
	m_builder.getInterface().m_nextChunkBias = nextChunkBias;
}

void StackMemoryPool::destroy()
{
	m_builder.destroy();
	m_builder.getInterface() = {};
	BaseMemoryPool::destroy();
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

} // end namespace anki
