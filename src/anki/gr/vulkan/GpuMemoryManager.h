// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/common/ClassAllocator.h>
#include <anki/gr/vulkan/Common.h>

namespace anki
{

/// @addtorgoup vulkan
/// @{

/// The handle that is returned from GpuMemoryManager's allocations.
class GpuMemoryHandle : public ClassAllocatorHandle
{
public:
	VkDeviceMemory m_memory = VK_NULL_HANDLE;
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

private:
	class Memory;
	class MemoryTypeCommon;
	class Interface;

	GrAllocator<U8> m_alloc;
	VkDevice m_dev;
	DynamicArray<MemoryTypeCommon> m_memType;
	VkPhysicalDeviceMemoryProperties m_memoryProperties;
};
/// @}

} // end namespace anki
