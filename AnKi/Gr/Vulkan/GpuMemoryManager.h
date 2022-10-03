// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/Common.h>
#include <AnKi/Util/ClassAllocatorBuilder.h>
#include <AnKi/Util/List.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

// Forward
class GpuMemoryManager;

/// @addtogroup vulkan
/// @{

class GpuMemoryManagerClassInfo
{
public:
	PtrSize m_suballocationSize;
	PtrSize m_chunkSize;
};

/// Implements the interface required by ClassAllocatorBuilder.
/// @memberof GpuMemoryManager
class GpuMemoryManagerChunk : public IntrusiveListEnabled<GpuMemoryManagerChunk>
{
public:
	VkDeviceMemory m_handle = VK_NULL_HANDLE;

	void* m_mappedAddress = nullptr;
	SpinLock m_mappedAddressMtx;

	PtrSize m_size = 0;

	// Bellow is the interface of ClassAllocatorBuilder

	BitSet<128, U64> m_inUseSuballocations = {false};
	U32 m_suballocationCount;
	void* m_class;
};

/// Implements the interface required by ClassAllocatorBuilder.
/// @memberof GpuMemoryManager
class GpuMemoryManagerInterface
{
public:
	GpuMemoryManager* m_parent = nullptr;

	U8 m_memTypeIdx = kMaxU8;
	Bool m_exposesBufferGpuAddress = false;

	Bool m_isDeviceMemory = false;

	PtrSize m_allocatedMemory = 0;
	PtrSize m_usedMemory = 0;

	ConstWeakArray<GpuMemoryManagerClassInfo> m_classInfos;

	// Bellow is the interface of ClassAllocatorBuilder

	U32 getClassCount() const
	{
		return m_classInfos.getSize();
	}

	void getClassInfo(U32 classIdx, PtrSize& chunkSize, PtrSize& suballocationSize) const
	{
		chunkSize = m_classInfos[classIdx].m_chunkSize;
		suballocationSize = m_classInfos[classIdx].m_suballocationSize;
	}

	Error allocateChunk(U32 classIdx, GpuMemoryManagerChunk*& chunk);

	void freeChunk(GpuMemoryManagerChunk* out);
};

/// The handle that is returned from GpuMemoryManager's allocations.
class GpuMemoryHandle
{
	friend class GpuMemoryManager;

public:
	VkDeviceMemory m_memory = VK_NULL_HANDLE;
	PtrSize m_offset = kMaxPtrSize;

	explicit operator Bool() const
	{
		return m_memory != VK_NULL_HANDLE && m_offset < kMaxPtrSize && m_memTypeIdx < kMaxU8;
	}

private:
	GpuMemoryManagerChunk* m_chunk = nullptr;
	PtrSize m_size = kMaxPtrSize;
	U8 m_memTypeIdx = kMaxU8;

	Bool isDedicated() const
	{
		return m_chunk == nullptr;
	}
};

/// @memberof GpuMemoryManager
class GpuMemoryManagerStats
{
public:
	PtrSize m_deviceMemoryAllocated;
	PtrSize m_deviceMemoryInUse;
	U32 m_deviceMemoryAllocationCount;
	PtrSize m_hostMemoryAllocated;
	PtrSize m_hostMemoryInUse;
	U32 m_hostMemoryAllocationCount;
};

/// Dynamic GPU memory allocator for all types.
class GpuMemoryManager
{
	friend class GpuMemoryManagerInterface;

public:
	GpuMemoryManager() = default;

	GpuMemoryManager(const GpuMemoryManager&) = delete; // Non-copyable

	~GpuMemoryManager();

	GpuMemoryManager& operator=(const GpuMemoryManager&) = delete; // Non-copyable

	void init(VkPhysicalDevice pdev, VkDevice dev, HeapMemoryPool* pool, Bool exposeBufferGpuAddress);

	void destroy();

	/// Allocate memory.
	void allocateMemory(U32 memTypeIdx, PtrSize size, U32 alignment, GpuMemoryHandle& handle);

	void allocateMemoryDedicated(U32 memTypeIdx, PtrSize size, VkImage image, GpuMemoryHandle& handle);

	/// Free memory.
	void freeMemory(GpuMemoryHandle& handle);

	/// Map memory.
	[[nodiscard]] void* getMappedAddress(GpuMemoryHandle& handle);

	/// Find a suitable memory type.
	U32 findMemoryType(U32 resourceMemTypeBits, VkMemoryPropertyFlags preferFlags,
					   VkMemoryPropertyFlags avoidFlags) const;

	/// Get some statistics.
	void getStats(GpuMemoryManagerStats& stats) const;

private:
	using ClassAllocator = ClassAllocatorBuilder<GpuMemoryManagerChunk, GpuMemoryManagerInterface, Mutex>;

	HeapMemoryPool* m_pool = nullptr;

	VkDevice m_dev = VK_NULL_HANDLE;

	DynamicArray<ClassAllocator> m_callocs;

	VkPhysicalDeviceMemoryProperties m_memoryProperties = {};
	U32 m_bufferImageGranularity = 0;

	// Dedicated allocation stats
	Atomic<PtrSize> m_dedicatedAllocatedMemory = {0};
	Atomic<U32> m_dedicatedAllocationCount = {0};
};
/// @}

} // end namespace anki
