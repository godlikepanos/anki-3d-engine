// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/CVarSet.h>
#include <AnKi/Gr/Buffer.h>
#include <AnKi/Util/SegregatedListsAllocatorBuilder.h>
#include <AnKi/Util/BlockArray.h>
#include <AnKi/Core/Common.h>

namespace anki {

ANKI_CVAR2(NumericCVar<PtrSize>, Core, TextureMemoryPool, ChunkSize, 256_MB, 16_MB, 1_GB, "Texture memory pool is allocated in chunks of this size")
ANKI_CVAR2(NumericCVar<PtrSize>, Core, TextureMemoryPool, MaxChunks, 4, 1, 32, "Max number of chunks")

class TextureMemoryPoolAllocation
{
	friend class TextureMemoryPool;

public:
	TextureMemoryPoolAllocation() = default;

	TextureMemoryPoolAllocation(const TextureMemoryPoolAllocation&) = delete;

	TextureMemoryPoolAllocation(TextureMemoryPoolAllocation&& b)
	{
		*this = std::move(b);
	}

	~TextureMemoryPoolAllocation();

	TextureMemoryPoolAllocation& operator=(const TextureMemoryPoolAllocation&) = delete;

	TextureMemoryPoolAllocation& operator=(TextureMemoryPoolAllocation&& b)
	{
		ANKI_ASSERT(!(*this) && "Need to manually free this");
		m_chunk = b.m_chunk;
		m_offset = b.m_offset;
		m_size = b.m_size;
		b.reset();
		return *this;
	}

	explicit operator Bool() const
	{
		return m_size > 0;
	}

	operator BufferView() const;

private:
	void* m_chunk = nullptr; // Type is TextureMemoryPool::SLChunk
	PtrSize m_offset = kMaxPtrSize;
	PtrSize m_size = 0;

	void reset()
	{
		m_chunk = nullptr;
		m_offset = kMaxPtrSize;
		m_size = 0;
	}
};

// It's a texture memory allocator. It allocates Buffers and sub-allocates from them.
class TextureMemoryPool : public MakeSingleton<TextureMemoryPool>
{
	friend class TextureMemoryPoolAllocation;

public:
	TextureMemoryPool();

	~TextureMemoryPool();

	// It's thread-safe
	TextureMemoryPoolAllocation allocate(PtrSize textureSize);

	// It's thread-safe
	void deferredFree(TextureMemoryPoolAllocation& alloc);

	void endFrame(Fence* fence);

private:
	class SLChunk;
	class SLInterface;
	using SLBuilder = SegregatedListsAllocatorBuilder<SLChunk, SLInterface, DummyMutex, SingletonMemoryPoolWrapper<CoreMemoryPool>>;

	class Garbage
	{
	public:
		CoreBlockArray<TextureMemoryPoolAllocation, BlockArrayConfig<16>> m_allocations;
		FencePtr m_fence;
	};

	SLBuilder* m_builder = nullptr;
	Mutex m_mtx;

	PtrSize m_allocatedSize = 0;

	CoreBlockArray<Garbage, BlockArrayConfig<8>> m_garbage;
	U32 m_activeGarbage = kMaxU32;

	U32 m_chunksCreated = 0;

	Error allocateChunk(SLChunk*& newChunk, PtrSize& chunkSize);
	void deleteChunk(SLChunk* chunk);

	void throwGarbage(Bool all);
};

inline TextureMemoryPoolAllocation::~TextureMemoryPoolAllocation()
{
	TextureMemoryPool::getSingleton().deferredFree(*this);
}

} // end namespace anki
