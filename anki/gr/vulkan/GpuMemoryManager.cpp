// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/GpuMemoryManager.h>
#include <anki/util/List.h>

namespace anki
{

constexpr U32 CLASS_COUNT = 7;

class ClassInf
{
public:
	PtrSize m_slotSize;
	PtrSize m_chunkSize;
};

static const Array<ClassInf, CLASS_COUNT> CLASSES{{{256_B, 16_KB},
												   {4_KB, 256_KB},
												   {128_KB, 8_MB},
												   {1_MB, 64_MB},
												   {16_MB, 128_MB},
												   {64_MB, 256_MB},
												   {128_MB, 256_MB}}};

class GpuMemoryManager::Memory final :
	public ClassGpuAllocatorMemory,
	public IntrusiveListEnabled<GpuMemoryManager::Memory>
{
public:
	VkDeviceMemory m_handle = VK_NULL_HANDLE;

	void* m_mappedAddress = nullptr;
	SpinLock m_mtx;

	U8 m_classIdx = MAX_U8;
};

class GpuMemoryManager::Interface final : public ClassGpuAllocatorInterface
{
public:
	GrAllocator<U8> m_alloc;
	Array<IntrusiveList<Memory>, CLASS_COUNT> m_vacantMemory;
	Mutex m_mtx;
	VkDevice m_dev = VK_NULL_HANDLE;
	U8 m_memTypeIdx = MAX_U8;
	Bool m_exposesBufferGpuAddress = false;

	Error allocate(U32 classIdx, ClassGpuAllocatorMemory*& cmem) override
	{
		Memory* mem;

		LockGuard<Mutex> lock(m_mtx);

		if(!m_vacantMemory[classIdx].isEmpty())
		{
			// Recycle
			mem = &m_vacantMemory[classIdx].getFront();
			m_vacantMemory[classIdx].popFront();
		}
		else
		{
			// Create new
			mem = m_alloc.newInstance<Memory>();

			VkMemoryAllocateInfo ci = {};
			ci.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			ci.allocationSize = CLASSES[classIdx].m_chunkSize;
			ci.memoryTypeIndex = m_memTypeIdx;

			VkMemoryAllocateFlagsInfo flags = {};
			flags.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
			flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
			if(m_exposesBufferGpuAddress)
			{
				ci.pNext = &flags;
			}

			ANKI_VK_CHECKF(vkAllocateMemory(m_dev, &ci, nullptr, &mem->m_handle));

			mem->m_classIdx = U8(classIdx);
		}

		ANKI_ASSERT(mem);
		ANKI_ASSERT(mem->m_handle);
		ANKI_ASSERT(mem->m_classIdx == classIdx);
		ANKI_ASSERT(mem->m_mappedAddress == nullptr);
		cmem = mem;

		return Error::NONE;
	}

	void free(ClassGpuAllocatorMemory* cmem) override
	{
		ANKI_ASSERT(cmem);

		Memory* mem = static_cast<Memory*>(cmem);
		ANKI_ASSERT(mem->m_handle);

		LockGuard<Mutex> lock(m_mtx);
		m_vacantMemory[mem->m_classIdx].pushBack(mem);

		// Unmap
		if(mem->m_mappedAddress)
		{
			vkUnmapMemory(m_dev, mem->m_handle);
			mem->m_mappedAddress = nullptr;
		}
	}

	U32 getClassCount() const override
	{
		return CLASS_COUNT;
	}

	void getClassInfo(U32 classIdx, PtrSize& slotSize, PtrSize& chunkSize) const override
	{
		slotSize = CLASSES[classIdx].m_slotSize;
		chunkSize = CLASSES[classIdx].m_chunkSize;
	}

	void collectGarbage()
	{
		LockGuard<Mutex> lock(m_mtx);

		for(U classIdx = 0; classIdx < CLASS_COUNT; ++classIdx)
		{
			while(!m_vacantMemory[classIdx].isEmpty())
			{
				Memory* mem = &m_vacantMemory[classIdx].getFront();
				m_vacantMemory[classIdx].popFront();

				if(mem->m_mappedAddress)
				{
					vkUnmapMemory(m_dev, mem->m_handle);
				}

				vkFreeMemory(m_dev, mem->m_handle, nullptr);

				m_alloc.deleteInstance(mem);
			}
		}
	}

	// Map memory
	void* mapMemory(ClassGpuAllocatorMemory* cmem)
	{
		ANKI_ASSERT(cmem);
		Memory* mem = static_cast<Memory*>(cmem);
		void* out;

		LockGuard<SpinLock> lock(mem->m_mtx);
		if(mem->m_mappedAddress)
		{
			out = mem->m_mappedAddress;
		}
		else
		{
			ANKI_VK_CHECKF(vkMapMemory(m_dev, mem->m_handle, 0, CLASSES[mem->m_classIdx].m_chunkSize, 0, &out));
			mem->m_mappedAddress = out;
		}

		ANKI_ASSERT(out);
		return out;
	}
};

class GpuMemoryManager::ClassAllocator : public ClassGpuAllocator
{
public:
	Bool m_isDeviceMemory;
};

GpuMemoryManager::~GpuMemoryManager()
{
}

void GpuMemoryManager::destroy()
{
	for(U32 i = 0; i < m_ifaces.getSize(); ++i)
	{
		for(U32 j = 0; j < 2; j++)
		{
			m_ifaces[i][j].collectGarbage();
		}
	}

	m_ifaces.destroy(m_alloc);
	m_callocs.destroy(m_alloc);
}

void GpuMemoryManager::init(VkPhysicalDevice pdev, VkDevice dev, GrAllocator<U8> alloc, Bool exposeBufferGpuAddress)
{
	ANKI_ASSERT(pdev);
	ANKI_ASSERT(dev);

	// Print some info
	ANKI_VK_LOGI("Initializing memory manager");
	for(const ClassInf& c : CLASSES)
	{
		ANKI_VK_LOGI("\tGPU mem class. Chunk size: %lu, slotSize: %lu, allocsPerChunk %lu", c.m_chunkSize, c.m_slotSize,
					 c.m_chunkSize / c.m_slotSize);
	}

	vkGetPhysicalDeviceMemoryProperties(pdev, &m_memoryProperties);

	m_alloc = alloc;

	m_ifaces.create(alloc, m_memoryProperties.memoryTypeCount);
	for(U32 memTypeIdx = 0; memTypeIdx < m_ifaces.getSize(); ++memTypeIdx)
	{
		for(U32 linear = 0; linear < 2; ++linear)
		{
			m_ifaces[memTypeIdx][linear].m_alloc = alloc;
			m_ifaces[memTypeIdx][linear].m_dev = dev;
			m_ifaces[memTypeIdx][linear].m_memTypeIdx = U8(memTypeIdx);
			m_ifaces[memTypeIdx][linear].m_exposesBufferGpuAddress = (linear == 1) && exposeBufferGpuAddress;
		}
	}

	// One allocator per linear/non-linear resources
	m_callocs.create(alloc, m_memoryProperties.memoryTypeCount);
	for(U32 memTypeIdx = 0; memTypeIdx < m_callocs.getSize(); ++memTypeIdx)
	{
		for(U32 linear = 0; linear < 2; ++linear)
		{
			m_callocs[memTypeIdx][linear].init(m_alloc, &m_ifaces[memTypeIdx][linear]);

			const U32 heapIdx = m_memoryProperties.memoryTypes[memTypeIdx].heapIndex;
			m_callocs[memTypeIdx][linear].m_isDeviceMemory =
				!!(m_memoryProperties.memoryHeaps[heapIdx].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);
		}
	}
}

void GpuMemoryManager::allocateMemory(U32 memTypeIdx, PtrSize size, U32 alignment, Bool linearResource,
									  GpuMemoryHandle& handle)
{
	ClassGpuAllocator& calloc = m_callocs[memTypeIdx][linearResource];
	const Error err = calloc.allocate(size, alignment, handle.m_classHandle);
	(void)err;

	handle.m_memory = static_cast<Memory*>(handle.m_classHandle.m_memory)->m_handle;
	handle.m_offset = handle.m_classHandle.m_offset;
	handle.m_linear = linearResource;
	handle.m_memTypeIdx = U8(memTypeIdx);
}

void GpuMemoryManager::freeMemory(GpuMemoryHandle& handle)
{
	ANKI_ASSERT(handle);
	ClassGpuAllocator& calloc = m_callocs[handle.m_memTypeIdx][handle.m_linear];
	calloc.free(handle.m_classHandle);

	handle = {};
}

void* GpuMemoryManager::getMappedAddress(GpuMemoryHandle& handle)
{
	ANKI_ASSERT(handle);

	Interface& iface = m_ifaces[handle.m_memTypeIdx][handle.m_linear];
	U8* out = static_cast<U8*>(iface.mapMemory(handle.m_classHandle.m_memory));
	return static_cast<void*>(out + handle.m_offset);
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
					// On some Intel drivers there are identical memory types pointing to different heaps. Chose the
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
			if(m_callocs[memTypeIdx][linear].m_isDeviceMemory)
			{
				gpuMemory += m_callocs[memTypeIdx][linear].getAllocatedMemory();
			}
			else
			{
				cpuMemory += m_callocs[memTypeIdx][linear].getAllocatedMemory();
			}
		}
	}
}

} // end namespace anki
