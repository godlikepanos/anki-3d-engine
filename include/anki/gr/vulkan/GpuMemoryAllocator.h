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
class GpuMemoryAllocatorClass;

/// @addtorgoup vulkan
/// @{

/// The handle that is returned from GpuMemoryAllocator's allocations.
class GpuMemoryAllocationHandle
{
	friend class GpuMemoryAllocator;

public:
	VkDeviceMemory m_memory = VK_NULL_HANDLE;
	PtrSize m_offset = MAX_PTR_SIZE;

	Bool isEmpty() const
	{
		return m_memory == VK_NULL_HANDLE;
	}

private:
	GpuMemoryAllocatorChunk* m_chunk = nullptr;
};

/// Dynamic GPU memory allocator.
class GpuMemoryAllocator
{
public:
	GpuMemoryAllocator()
	{
	}

	~GpuMemoryAllocator();

	void init(
		GenericMemoryPoolAllocator<U8> alloc, VkDevice dev, U memoryTypeIdx);

	/// Allocate GPU memory.
	void allocate(PtrSize size, U alignment, GpuMemoryAllocationHandle& handle);

	/// Free allocated memory.
	void free(GpuMemoryAllocationHandle& handle);

	/// Get CPU visible address.
	void* getMappedAddress(GpuMemoryAllocationHandle& handle);

private:
	using Class = GpuMemoryAllocatorClass;
	using Chunk = GpuMemoryAllocatorChunk;

	GenericMemoryPoolAllocator<U8> m_alloc;

	VkDevice m_dev = VK_NULL_HANDLE;
	U32 m_memIdx;

	/// The memory classes.
	DynamicArray<Class> m_classes;

	Class* findClass(PtrSize size, U32 alignment);

	Chunk* findChunkWithUnusedSlot(Class& cl);

	/// Create or recycle chunk.
	Chunk& createChunk(Class& cl);

	/// Park the chunk.
	void destroyChunk(Class& cl, Chunk& chunk);
};
/// @}

} // end namespace anki