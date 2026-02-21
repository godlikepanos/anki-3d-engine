// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/GpuMemory/SegregatedListsGpuMemoryPool.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Gr/Buffer.h>
#include <AnKi/Gr/Fence.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Util/BlockArray.h>

namespace anki {

ANKI_CVAR2(NumericCVar<PtrSize>, GpuMem, TextureMemoryPool, ChunkSize, 256_MB, 16_MB, 1_GB, "Texture memory pool is allocated in chunks of this size")
ANKI_CVAR2(NumericCVar<U32>, GpuMem, TextureMemoryPool, MaxChunks, 4, 1, 32, "Max number of chunks")

ANKI_SVAR(TextureMemoryPoolCapacity, StatCategory::kGpuMem, "Texture mem pool total size", StatFlag::kBytes | StatFlag::kMainThreadUpdates)
ANKI_SVAR(TextureMemoryPoolUsedMemory, StatCategory::kGpuMem, "Texture mem in use", StatFlag::kBytes | StatFlag::kMainThreadUpdates)

using TextureMemoryPoolAllocation = SegregatedListsGpuMemoryPoolAllocation;

// It's primarely a texture memory allocator. Sometimes we allocate non-texture memory though. It allocates Buffers and sub-allocates from them.
class TextureMemoryPool : public MakeSingleton<TextureMemoryPool>
{
public:
	TextureMemoryPool()
	{
		const Array<PtrSize, 8> memoryClasses = {256_KB, 1_MB, 4_MB, 8_MB, 16_MB, 32_MB, 128_MB, 256_MB};
		m_pool.init(g_cvarGpuMemTextureMemoryPoolChunkSize, g_cvarGpuMemTextureMemoryPoolMaxChunks, "TexPool", memoryClasses,
					BufferUsageBit::kTexture | BufferUsageBit::kAllSrv | BufferUsageBit::kAllCopy | BufferUsageBit::kVertexOrIndex
						| BufferUsageBit::kAllIndirect | BufferUsageBit::kAllUav);
	}

	~TextureMemoryPool() = default;

	// Generic allocation
	// It's thread-safe
	TextureMemoryPoolAllocation allocate(PtrSize size, U32 alignment)
	{
		return m_pool.allocate(size, alignment);
	}

	// Texture memory allocation. Supply the size returned by GrManager::getTextureMemoryRequirement
	// It's thread-safe
	TextureMemoryPoolAllocation allocate(PtrSize textureSize)
	{
		return m_pool.allocate(textureSize, 1);
	}

	// It's thread-safe
	template<typename T>
	TextureMemoryPoolAllocation allocateStructuredBuffer(U32 count)
	{
		return m_pool.allocateStructuredBuffer<T>(count);
	}

	// Allocate a vertex buffer.
	// It's thread-safe
	TextureMemoryPoolAllocation allocateFormat(Format format, U32 count)
	{
		const U32 texelSize = getFormatInfo(format).m_texelSize;
		return allocate(texelSize * count, texelSize);
	}

	// It's thread-safe
	void deferredFree(TextureMemoryPoolAllocation& alloc)
	{
		m_pool.deferredFree(alloc);
	}

	void endFrame(Fence* fence)
	{
		m_pool.endFrame(fence);
		PtrSize allocatedSize;
		PtrSize memoryCapacity;
		m_pool.getStats(allocatedSize, memoryCapacity);
		g_svarTextureMemoryPoolCapacity.set(memoryCapacity);
		g_svarTextureMemoryPoolUsedMemory.set(allocatedSize);
	}

private:
	SegregatedListsGpuMemoryPool m_pool;
};

} // end namespace anki
