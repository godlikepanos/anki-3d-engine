// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/Common.h>
#include <anki/util/List.h>

namespace anki
{

// Forward
class GpuMemoryAllocatorChunk;

/// @addtorgoup vulkan
/// @{

/// The handle that is returned from GpuMemoryAllocator's allocations.
class GpuMemoryAllocationHandle
{
	friend class GpuMemoryAllocator;

public:
	VkDeviceMemory m_memory;
	PtrSize m_offset;

private:
	GpuMemoryAllocatorChunk* m_chunk;
};

/// GPU memory allocator.
class GpuMemoryAllocator
{
public:
	GpuMemoryAllocator()
	{
	}

	~GpuMemoryAllocator();

	void init(GenericMemoryPoolAllocator<U8> alloc,
		VkDevice dev,
		U memoryTypeIdx,
		PtrSize chunkInitialSize,
		F32 nextChunkScale,
		PtrSize nextChunkBias);

	/// Allocate GPU memory.
	void allocate(PtrSize size, U alignment, GpuMemoryAllocationHandle& handle);

	/// Free allocated memory.
	void free(const GpuMemoryAllocationHandle& handle);

private:
	using Chunk = GpuMemoryAllocatorChunk;

	GenericMemoryPoolAllocator<U8> m_alloc;

	VkDevice m_dev = VK_NULL_HANDLE;
	U32 m_memIdx;

	IntrusiveList<Chunk> m_activeChunks;
	Mutex m_mtx;

	/// Size of the first chunk.
	PtrSize m_initSize = 0;

	/// Chunk scale.
	F32 m_scale = 2.0;

	/// Chunk bias.
	PtrSize m_bias = 0;

	void createNewChunk();

	Bool allocateFromChunk(PtrSize size,
		U alignment,
		Chunk& ch,
		GpuMemoryAllocationHandle& handle);
};
/// @}

} // end namespace anki