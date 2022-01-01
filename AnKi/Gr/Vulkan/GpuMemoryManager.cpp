// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/GpuMemoryManager.h>

namespace anki {

static constexpr Array<GpuMemoryManagerClassInfo, 7> CLASSES{{{256_B, 16_KB},
															  {4_KB, 256_KB},
															  {128_KB, 8_MB},
															  {1_MB, 64_MB},
															  {16_MB, 128_MB},
															  {64_MB, 256_MB},
															  {128_MB, 256_MB}}};

/// Special classes for the ReBAR memory. Have that as a special case because it's so limited and needs special care.
static constexpr Array<GpuMemoryManagerClassInfo, 3> REBAR_CLASSES{{{1_MB, 1_MB}, {12_MB, 12_MB}, {24_MB, 24_MB}}};

Error GpuMemoryManagerInterface::allocateChunk(U32 classIdx, GpuMemoryManagerChunk*& chunk)
{
	VkMemoryAllocateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	ci.allocationSize = m_classInfos[classIdx].m_chunkSize;
	ci.memoryTypeIndex = m_memTypeIdx;

	VkMemoryAllocateFlagsInfo flags = {};
	flags.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
	if(m_exposesBufferGpuAddress)
	{
		ci.pNext = &flags;
	}

	VkDeviceMemory memHandle;
	if(ANKI_UNLIKELY(vkAllocateMemory(m_parent->m_dev, &ci, nullptr, &memHandle) != VK_SUCCESS))
	{
		ANKI_VK_LOGF("Out of GPU memory. Mem type index %u, size %zu", m_memTypeIdx,
					 m_classInfos[classIdx].m_suballocationSize);
	}

	chunk = m_parent->m_alloc.newInstance<GpuMemoryManagerChunk>();
	chunk->m_handle = memHandle;
	chunk->m_size = m_classInfos[classIdx].m_chunkSize;

	m_allocatedMemory += m_classInfos[classIdx].m_chunkSize;

	return Error::NONE;
}

void GpuMemoryManagerInterface::freeChunk(GpuMemoryManagerChunk* chunk)
{
	ANKI_ASSERT(chunk);
	ANKI_ASSERT(chunk->m_handle != VK_NULL_HANDLE);

	if(chunk->m_mappedAddress)
	{
		vkUnmapMemory(m_parent->m_dev, chunk->m_handle);
	}

	vkFreeMemory(m_parent->m_dev, chunk->m_handle, nullptr);

	ANKI_ASSERT(m_allocatedMemory >= chunk->m_size);
	m_allocatedMemory -= chunk->m_size;

	m_parent->m_alloc.deleteInstance(chunk);
}

GpuMemoryManager::~GpuMemoryManager()
{
}

void GpuMemoryManager::destroy()
{
	m_callocs.destroy(m_alloc);
}

void GpuMemoryManager::init(VkPhysicalDevice pdev, VkDevice dev, GrAllocator<U8> alloc, Bool exposeBufferGpuAddress)
{
	ANKI_ASSERT(pdev);
	ANKI_ASSERT(dev);

	// Print some info
	ANKI_VK_LOGI("Initializing memory manager");
	for(const GpuMemoryManagerClassInfo& c : CLASSES)
	{
		ANKI_VK_LOGI("\tGPU mem class. Chunk size: %lu, suballocationSize: %lu, allocsPerChunk %lu", c.m_chunkSize,
					 c.m_suballocationSize, c.m_chunkSize / c.m_suballocationSize);
	}

	vkGetPhysicalDeviceMemoryProperties(pdev, &m_memoryProperties);

	m_alloc = alloc;
	m_dev = dev;

	m_callocs.create(alloc, m_memoryProperties.memoryTypeCount);
	for(U32 memTypeIdx = 0; memTypeIdx < m_callocs.getSize(); ++memTypeIdx)
	{
		for(U32 linear = 0; linear < 2; ++linear)
		{
			GpuMemoryManagerInterface& iface = m_callocs[memTypeIdx][linear].getInterface();
			iface.m_parent = this;
			iface.m_memTypeIdx = U8(memTypeIdx);
			iface.m_exposesBufferGpuAddress = (linear == 1) && exposeBufferGpuAddress;

			const U32 heapIdx = m_memoryProperties.memoryTypes[memTypeIdx].heapIndex;
			iface.m_isDeviceMemory =
				!!(m_memoryProperties.memoryHeaps[heapIdx].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);

			// Find if it's ReBAR
			const VkMemoryPropertyFlags props = m_memoryProperties.memoryTypes[memTypeIdx].propertyFlags;
			const VkMemoryPropertyFlags reBarProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
													 | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
													 | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			const PtrSize heapSize =
				m_memoryProperties.memoryHeaps[m_memoryProperties.memoryTypes[memTypeIdx].heapIndex].size;
			const Bool isReBar = props == reBarProps && heapSize <= 256_MB;

			if(isReBar)
			{
				ANKI_VK_LOGI("Memory type %u is ReBAR", memTypeIdx);
			}

			// Choose different classes
			if(!isReBar)
			{
				iface.m_classInfos = CLASSES;
			}
			else
			{
				iface.m_classInfos = REBAR_CLASSES;
			}

			// The interface is initialized, init the builder
			m_callocs[memTypeIdx][linear].init(m_alloc);
		}
	}
}

void GpuMemoryManager::allocateMemory(U32 memTypeIdx, PtrSize size, U32 alignment, Bool linearResource,
									  GpuMemoryHandle& handle)
{
	ClassAllocator& calloc = m_callocs[memTypeIdx][linearResource];

	GpuMemoryManagerChunk* chunk;
	PtrSize offset;
	const Error err = calloc.allocate(size, alignment, chunk, offset);
	(void)err;

	handle.m_memory = chunk->m_handle;
	handle.m_offset = offset;
	handle.m_chunk = chunk;
	handle.m_memTypeIdx = U8(memTypeIdx);
	handle.m_linear = linearResource;
}

void GpuMemoryManager::freeMemory(GpuMemoryHandle& handle)
{
	ANKI_ASSERT(handle);
	ClassAllocator& calloc = m_callocs[handle.m_memTypeIdx][handle.m_linear];

	calloc.free(handle.m_chunk, handle.m_offset);

	handle = {};
}

void* GpuMemoryManager::getMappedAddress(GpuMemoryHandle& handle)
{
	ANKI_ASSERT(handle);

	LockGuard<SpinLock> lock(handle.m_chunk->m_m_mappedAddressMtx);

	if(handle.m_chunk->m_mappedAddress == nullptr)
	{
		ANKI_VK_CHECKF(vkMapMemory(m_dev, handle.m_chunk->m_handle, 0, handle.m_chunk->m_size, 0,
								   &handle.m_chunk->m_mappedAddress));
	}

	return static_cast<void*>(static_cast<U8*>(handle.m_chunk->m_mappedAddress) + handle.m_offset);
}

U32 GpuMemoryManager::findMemoryType(U32 resourceMemTypeBits, VkMemoryPropertyFlags preferFlags,
									 VkMemoryPropertyFlags avoidFlags) const
{
	U32 prefered = MAX_U32;

	// Iterate all mem types
	for(U32 i = 0; i < m_memoryProperties.memoryTypeCount; i++)
	{
		if(resourceMemTypeBits & (1u << i))
		{
			const VkMemoryPropertyFlags flags = m_memoryProperties.memoryTypes[i].propertyFlags;

			if((flags & preferFlags) == preferFlags && (flags & avoidFlags) == 0)
			{
				// It's the candidate we want

				if(prefered == MAX_U32)
				{
					prefered = i;
				}
				else
				{
					// On some Intel drivers there are identical memory types pointing to different heaps. Choose the
					// biggest heap

					const PtrSize crntHeapSize =
						m_memoryProperties.memoryHeaps[m_memoryProperties.memoryTypes[i].heapIndex].size;
					const PtrSize prevHeapSize =
						m_memoryProperties.memoryHeaps[m_memoryProperties.memoryTypes[prefered].heapIndex].size;

					if(crntHeapSize > prevHeapSize)
					{
						prefered = i;
					}
				}
			}
		}
	}

	return prefered;
}

void GpuMemoryManager::getAllocatedMemory(PtrSize& gpuMemory, PtrSize& cpuMemory) const
{
	gpuMemory = 0;
	cpuMemory = 0;

	for(U32 memTypeIdx = 0; memTypeIdx < m_callocs.getSize(); ++memTypeIdx)
	{
		for(U32 linear = 0; linear < 2; ++linear)
		{
			const GpuMemoryManagerInterface& iface = m_callocs[memTypeIdx][linear].getInterface();
			if(iface.m_isDeviceMemory)
			{
				gpuMemory += iface.m_allocatedMemory;
			}
			else
			{
				cpuMemory += iface.m_allocatedMemory;
			}
		}
	}
}

} // end namespace anki
