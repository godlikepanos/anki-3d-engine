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
	SpinLock m_m_mappedAddressMtx;

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

	U8 m_memTypeIdx = MAX_U8;
	Bool m_exposesBufferGpuAddress = false;

	Bool m_isDeviceMemory = false;

	PtrSize m_allocatedMemory = 0;

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
	PtrSize m_offset = MAX_PTR_SIZE;

	explicit operator Bool() const
	{
		return m_memory != VK_NULL_HANDLE && m_offset < MAX_PTR_SIZE && m_memTypeIdx < MAX_U8;
	}

private:
	GpuMemoryManagerChunk* m_chunk = nullptr;
	U8 m_memTypeIdx = MAX_U8;
	Bool m_linear = false;
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

	void init(VkPhysicalDevice pdev, VkDevice dev, GrAllocator<U8> alloc, Bool exposeBufferGpuAddress);

	void destroy();

	/// Allocate memory.
	void allocateMemory(U32 memTypeIdx, PtrSize size, U32 alignment, Bool linearResource, GpuMemoryHandle& handle);

	/// Free memory.
	void freeMemory(GpuMemoryHandle& handle);

	/// Map memory.
	ANKI_USE_RESULT void* getMappedAddress(GpuMemoryHandle& handle);

	/// Find a suitable memory type.
	U32 findMemoryType(U32 resourceMemTypeBits, VkMemoryPropertyFlags preferFlags,
					   VkMemoryPropertyFlags avoidFlags) const;

	/// Get some statistics.
	void getAllocatedMemory(PtrSize& gpuMemory, PtrSize& cpuMemory) const;

private:
	using ClassAllocator = ClassAllocatorBuilder<GpuMemoryManagerChunk, GpuMemoryManagerInterface, Mutex>;

	GrAllocator<U8> m_alloc;

	VkDevice m_dev = VK_NULL_HANDLE;

	DynamicArray<Array<ClassAllocator, 2>> m_callocs;

	VkPhysicalDeviceMemoryProperties m_memoryProperties;
};
/// @}

} // end namespace anki
