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
	friend class GpuMemoryAllocatorMemType;

public:
	VkDeviceMemory m_memory = VK_NULL_HANDLE;
	PtrSize m_offset = MAX_PTR_SIZE;
	U32 m_memoryTypeIndex = MAX_U32;

	Bool isEmpty() const
	{
		return m_memory == VK_NULL_HANDLE;
	}

	operator bool() const
	{
		return m_memory != VK_NULL_HANDLE;
	}

private:
	GpuMemoryAllocatorChunk* m_chunk = nullptr;
};

/// Dynamic GPU memory allocator for a specific type.
class GpuMemoryAllocatorMemType
{
public:
	GpuMemoryAllocatorMemType()
	{
	}

	~GpuMemoryAllocatorMemType();

	void init(
		GenericMemoryPoolAllocator<U8> alloc, VkDevice dev, U memoryTypeIdx);

	/// Allocate GPU memory.
	void allocate(PtrSize size,
		U alignment,
		Bool linearResource,
		GpuMemoryAllocationHandle& handle);

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

	Chunk* findChunkWithUnusedSlot(Class& cl, Bool linearResource);

	/// Create or recycle chunk.
	Chunk& createChunk(Class& cl, Bool linearResources);

	/// Park the chunk.
	void destroyChunk(Class& cl, Chunk& chunk);
};

/// Dynamic GPU memory allocator for all types.
class GpuMemoryAllocator
{
public:
	GpuMemoryAllocator()
	{
	}

	~GpuMemoryAllocator();

	void init(VkPhysicalDevice pdev, VkDevice dev, GrAllocator<U8> alloc);

	void destroy();

	/// Allocate memory.
	void allocateMemory(U memTypeIdx,
		PtrSize size,
		U alignment,
		Bool linearResource,
		GpuMemoryAllocationHandle& handle)
	{
		m_gpuAllocs[memTypeIdx].allocate(
			size, alignment, linearResource, handle);
		handle.m_memoryTypeIndex = memTypeIdx;
	}

	/// Free memory.
	void freeMemory(GpuMemoryAllocationHandle& handle)
	{
		m_gpuAllocs[handle.m_memoryTypeIndex].free(handle);
	}

	/// Map memory.
	ANKI_USE_RESULT void* getMappedAddress(GpuMemoryAllocationHandle& handle)
	{
		return m_gpuAllocs[handle.m_memoryTypeIndex].getMappedAddress(handle);
	}

	/// Find a suitable memory type.
	U findMemoryType(U resourceMemTypeBits,
		VkMemoryPropertyFlags preferFlags,
		VkMemoryPropertyFlags avoidFlags) const;

private:
	/// One for each mem type.
	DynamicArray<GpuMemoryAllocatorMemType> m_gpuAllocs;
	VkPhysicalDeviceMemoryProperties m_memoryProperties;
	VkDevice m_dev;
	GrAllocator<U8> m_alloc;
};
/// @}

} // end namespace anki
