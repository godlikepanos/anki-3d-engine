// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/common/ClassGpuAllocator.h>
#include <anki/gr/vulkan/Common.h>

namespace anki
{

/// @addtorgoup vulkan
/// @{

/// The handle that is returned from GpuMemoryManager's allocations.
class GpuMemoryHandle
{
	friend class GpuMemoryManager;

public:
	VkDeviceMemory m_memory = VK_NULL_HANDLE;
	PtrSize m_offset = MAX_PTR_SIZE;

	operator Bool() const
	{
		return m_memory != VK_NULL_HANDLE && m_offset < MAX_PTR_SIZE && m_memTypeIdx < MAX_U8;
	}

private:
	ClassGpuAllocatorHandle m_classHandle;
	U8 m_memTypeIdx = MAX_U8;
	Bool8 m_linear = false;
};

/// Dynamic GPU memory allocator for all types.
class GpuMemoryManager : public NonCopyable
{
public:
	GpuMemoryManager() = default;

	~GpuMemoryManager();

	void init(VkPhysicalDevice pdev, VkDevice dev, GrAllocator<U8> alloc);

	void destroy();

	/// Allocate memory.
	void allocateMemory(U memTypeIdx, PtrSize size, U alignment, Bool linearResource, GpuMemoryHandle& handle);

	/// Free memory.
	void freeMemory(GpuMemoryHandle& handle);

	/// Map memory.
	ANKI_USE_RESULT void* getMappedAddress(GpuMemoryHandle& handle);

	/// Find a suitable memory type.
	U findMemoryType(U resourceMemTypeBits, VkMemoryPropertyFlags preferFlags, VkMemoryPropertyFlags avoidFlags) const;

	/// Get some statistics.
	void getAllocatedMemory(PtrSize& gpuMemory, PtrSize& cpuMemory) const;

private:
	class Memory;
	class Interface;
	class ClassAllocator;

	GrAllocator<U8> m_alloc;
	VkDevice m_dev;
	DynamicArray<Interface> m_ifaces;
	DynamicArray<ClassAllocator> m_callocs;
	VkPhysicalDeviceMemoryProperties m_memoryProperties;
};
/// @}

} // end namespace anki
