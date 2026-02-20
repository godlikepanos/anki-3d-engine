// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/GpuMemory/TextureMemoryPool.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/Fence.h>
#include <AnKi/Core/StatsSet.h>

namespace anki {

ANKI_SVAR(TextureMemoryPoolCapacity, StatCategory::kGpuMem, "Texture mem pool total size", StatFlag::kBytes | StatFlag::kMainThreadUpdates)
ANKI_SVAR(TextureMemoryPoolUsedMemory, StatCategory::kGpuMem, "Texture mem in use", StatFlag::kBytes | StatFlag::kMainThreadUpdates)

static constexpr Array<PtrSize, 8> kMemoryClasses = {256_KB, 1_MB, 4_MB, 8_MB, 16_MB, 32_MB, 128_MB, 256_MB};

class TextureMemoryPool::SLChunk : public SegregatedListsAllocatorBuilderChunkBase<SingletonMemoryPoolWrapper<DefaultMemoryPool>>
{
public:
	BufferPtr m_buffer;
};

class TextureMemoryPool::SLInterface
{
public:
	// Interface methods
	U32 getClassCount() const
	{
		return kMemoryClasses.getSize();
	}

	void getClassInfo(U32 idx, PtrSize& size) const
	{
		size = kMemoryClasses[idx];
	}

	Error allocateChunk(SLChunk*& newChunk, PtrSize& chunkSize)
	{
		return TextureMemoryPool::getSingleton().allocateChunk(newChunk, chunkSize);
	}

	void deleteChunk(SLChunk* chunk)
	{
		TextureMemoryPool::getSingleton().deleteChunk(chunk);
	}

	static constexpr PtrSize getMinSizeAlignment()
	{
		return 4;
	}
};

TextureMemoryPool::TextureMemoryPool()
{
	m_builder = newInstance<SLBuilder>(DefaultMemoryPool::getSingleton());

	if(!GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferNaturalAlignment)
	{
		m_structuredBufferBindOffsetAlignment = U16(GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferBindOffsetAlignment);
	}
}

TextureMemoryPool::~TextureMemoryPool()
{
	GrManager::getSingleton().finish();

	throwGarbage(true);

	deleteInstance(DefaultMemoryPool::getSingleton(), m_builder);
}

Error TextureMemoryPool::allocateChunk(SLChunk*& newChunk, PtrSize& chunkSize)
{
	if(TextureMemoryPool::getSingleton().m_chunksCreated == g_cvarGpuMemTextureMemoryPoolMaxChunks)
	{
		ANKI_GPUMEM_LOGE("Reached the max limit of memory chunks. Incrase %s", g_cvarGpuMemTextureMemoryPoolMaxChunks.getName().cstr());
		return Error::kOutOfMemory;
	}

	BufferInitInfo buffInit("TexPoolChunk");
	buffInit.m_size = g_cvarGpuMemTextureMemoryPoolChunkSize;
	buffInit.m_usage = BufferUsageBit::kTexture | BufferUsageBit::kAllSrv | BufferUsageBit::kAllCopy | BufferUsageBit::kVertexOrIndex
					   | BufferUsageBit::kAllIndirect | BufferUsageBit::kAllUav;

	BufferPtr buff = GrManager::getSingleton().newBuffer(buffInit);

	newChunk = newInstance<SLChunk>(DefaultMemoryPool::getSingleton());
	newChunk->m_buffer = buff;

	chunkSize = g_cvarGpuMemTextureMemoryPoolChunkSize;

	++TextureMemoryPool::getSingleton().m_chunksCreated;

	g_svarTextureMemoryPoolCapacity.increment(U32(g_cvarGpuMemTextureMemoryPoolChunkSize));

	return Error::kNone;
}

void TextureMemoryPool::deleteChunk(SLChunk* chunk)
{
	if(chunk)
	{
		ANKI_GPUMEM_LOGW("Will delete texture mem pool chunk and will serialize CPU and GPU");
		GrManager::getSingleton().finish();
		deleteInstance(DefaultMemoryPool::getSingleton(), chunk);

		// Wait again to force delete the memory
		GrManager::getSingleton().finish();

		ANKI_ASSERT(TextureMemoryPool::getSingleton().m_chunksCreated > 0);
		--TextureMemoryPool::getSingleton().m_chunksCreated;

		g_svarTextureMemoryPoolCapacity.decrement(U32(g_cvarGpuMemTextureMemoryPoolChunkSize));
	}
}

TextureMemoryPoolAllocation TextureMemoryPool::allocate(PtrSize size, U32 alignment)
{
	LockGuard lock(m_mtx);

	SLChunk* chunk;
	PtrSize offset;
	if(m_builder->allocate(size, alignment, chunk, offset))
	{
		ANKI_GPUMEM_LOGF("Failed to allocate tex memory");
	}

	TextureMemoryPoolAllocation alloc;
	alloc.m_chunk = chunk;
	alloc.m_offset = offset;
	alloc.m_size = size;

	m_allocatedSize += size;

	return alloc;
}

void TextureMemoryPool::deferredFree(TextureMemoryPoolAllocation& alloc)
{
	if(!alloc)
	{
		return;
	}

	LockGuard lock(m_mtx);

	if(m_activeGarbage == kMaxU32)
	{
		m_activeGarbage = m_garbage.emplace().getArrayIndex();
	}

	m_garbage[m_activeGarbage].m_allocations.emplace(std::move(alloc));
	ANKI_ASSERT(!alloc);
}

void TextureMemoryPool::endFrame(Fence* fence)
{
	ANKI_ASSERT(fence);

	LockGuard lock(m_mtx);

	throwGarbage(false);

	// Set the new fence
	if(m_activeGarbage != kMaxU32)
	{
		ANKI_ASSERT(m_garbage[m_activeGarbage].m_allocations.getSize());
		ANKI_ASSERT(!m_garbage[m_activeGarbage].m_fence);
		m_garbage[m_activeGarbage].m_fence.reset(fence);

		m_activeGarbage = kMaxU32;
	}

	g_svarTextureMemoryPoolUsedMemory.set(m_allocatedSize);
}

void TextureMemoryPool::throwGarbage(Bool all)
{
	Array<U32, 8> garbageToDelete;
	U32 garbageToDeleteCount = 0;
	for(auto it = m_garbage.getBegin(); it != m_garbage.getEnd(); ++it)
	{
		Garbage& garbage = *it;

		if(all || (garbage.m_fence && garbage.m_fence->signaled()))
		{
			for(TextureMemoryPoolAllocation& alloc : garbage.m_allocations)
			{
				m_builder->free(static_cast<SLChunk*>(alloc.m_chunk), alloc.m_offset, alloc.m_size);

				ANKI_ASSERT(m_allocatedSize >= alloc.m_size);
				m_allocatedSize -= alloc.m_size;

				alloc.reset();
			}

			garbageToDelete[garbageToDeleteCount++] = it.getArrayIndex();
		}
	}

	for(U32 i = 0; i < garbageToDeleteCount; ++i)
	{
		m_garbage.erase(garbageToDelete[i]);
	}
}

TextureMemoryPoolAllocation::operator BufferView() const
{
	ANKI_ASSERT(!!(*this));
	TextureMemoryPool::SLChunk* chunk = static_cast<TextureMemoryPool::SLChunk*>(m_chunk);

	return BufferView(chunk->m_buffer.get(), m_offset, m_size);
}

} // end namespace anki
