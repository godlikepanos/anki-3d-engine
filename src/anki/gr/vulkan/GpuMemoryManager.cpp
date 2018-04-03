// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/GpuMemoryManager.h>
#include <anki/util/List.h>

namespace anki
{

const U CLASS_COUNT = 7;

class ClassInf
{
public:
	PtrSize m_slotSize;
	PtrSize m_chunkSize;
};

static const Array<ClassInf, CLASS_COUNT> CLASSES = {{{256_B, 16_KB},
	{4_KB, 256_KB},
	{128_KB, 8_MB},
	{1_MB, 64_MB},
	{16_MB, 128_MB},
	{64_MB, 256_MB},
	{128_MB, 256_MB}}};

class GpuMemoryManager::Memory final : public ClassGpuAllocatorMemory,
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

	Error allocate(U classIdx, ClassGpuAllocatorMemory*& cmem)
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
			ANKI_VK_CHECKF(vkAllocateMemory(m_dev, &ci, nullptr, &mem->m_handle));

			mem->m_classIdx = classIdx;
		}

		ANKI_ASSERT(mem);
		ANKI_ASSERT(mem->m_handle);
		ANKI_ASSERT(mem->m_classIdx == classIdx);
		ANKI_ASSERT(mem->m_mappedAddress == nullptr);
		cmem = mem;

		return Error::NONE;
	}

	void free(ClassGpuAllocatorMemory* cmem)
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

	U getClassCount() const
	{
		return CLASS_COUNT;
	}

	void getClassInfo(U classIdx, PtrSize& slotSize, PtrSize& chunkSize) const
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

	// Mapp memory
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
	Bool8 m_isDeviceMemory;
};

GpuMemoryManager::~GpuMemoryManager()
{
}

void GpuMemoryManager::destroy()
{
	for(Interface& iface : m_ifaces)
	{
		iface.collectGarbage();
	}

	m_ifaces.destroy(m_alloc);
	m_callocs.destroy(m_alloc);
}

void GpuMemoryManager::init(VkPhysicalDevice pdev, VkDevice dev, GrAllocator<U8> alloc)
{
	ANKI_ASSERT(pdev);
	ANKI_ASSERT(dev);

	// Print some info
	ANKI_VK_LOGI("Initializing memory manager");
	for(const ClassInf& c : CLASSES)
	{
		ANKI_VK_LOGI("\tGPU mem class. Chunk size: %u, slotSize: %u, allocsPerChunk %u",
			c.m_chunkSize,
			c.m_slotSize,
			c.m_chunkSize / c.m_slotSize);
	}

	vkGetPhysicalDeviceMemoryProperties(pdev, &m_memoryProperties);

	m_alloc = alloc;

	m_ifaces.create(alloc, m_memoryProperties.memoryTypeCount);
	for(U i = 0; i < m_ifaces.getSize(); ++i)
	{
		Interface& iface = m_ifaces[i];

		iface.m_alloc = alloc;
		iface.m_dev = dev;
		iface.m_memTypeIdx = i;
	}

	// One allocator per type per linear/non-linear resources
	m_callocs.create(alloc, m_memoryProperties.memoryTypeCount * 2);
	for(U i = 0; i < m_callocs.getSize(); ++i)
	{
		m_callocs[i].init(m_alloc, &m_ifaces[i / 2]);

		const U memTypeIdx = i / 2;
		const U heapIdx = m_memoryProperties.memoryTypes[memTypeIdx].heapIndex;
		m_callocs[i].m_isDeviceMemory =
			!!(m_memoryProperties.memoryHeaps[heapIdx].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);
	}
}

void GpuMemoryManager::allocateMemory(
	U memTypeIdx, PtrSize size, U alignment, Bool linearResource, GpuMemoryHandle& handle)
{
	ClassGpuAllocator& calloc = m_callocs[memTypeIdx * 2 + ((linearResource) ? 0 : 1)];
	Error err = calloc.allocate(size, alignment, handle.m_classHandle);
	(void)err;

	handle.m_memory = static_cast<Memory*>(handle.m_classHandle.m_memory)->m_handle;
	handle.m_offset = handle.m_classHandle.m_offset;
	handle.m_linear = linearResource;
	handle.m_memTypeIdx = memTypeIdx;
}

void GpuMemoryManager::freeMemory(GpuMemoryHandle& handle)
{
	ANKI_ASSERT(handle);

	ClassGpuAllocator& calloc = m_callocs[handle.m_memTypeIdx * 2 + ((handle.m_linear) ? 0 : 1)];
	calloc.free(handle.m_classHandle);

	handle = {};
}

void* GpuMemoryManager::getMappedAddress(GpuMemoryHandle& handle)
{
	ANKI_ASSERT(handle);

	Interface& iface = m_ifaces[handle.m_memTypeIdx];
	U8* out = static_cast<U8*>(iface.mapMemory(handle.m_classHandle.m_memory));
	return static_cast<void*>(out + handle.m_offset);
}

U GpuMemoryManager::findMemoryType(
	U resourceMemTypeBits, VkMemoryPropertyFlags preferFlags, VkMemoryPropertyFlags avoidFlags) const
{
	U prefered = MAX_U32;

	// Iterate all mem types
	for(U i = 0; i < m_memoryProperties.memoryTypeCount; i++)
	{
		if(resourceMemTypeBits & (1u << i))
		{
			VkMemoryPropertyFlags flags = m_memoryProperties.memoryTypes[i].propertyFlags;

			if((flags & preferFlags) == preferFlags && (flags & avoidFlags) == 0)
			{
				prefered = i;
			}
		}
	}

	return prefered;
}

void GpuMemoryManager::getAllocatedMemory(PtrSize& gpuMemory, PtrSize& cpuMemory) const
{
	gpuMemory = 0;
	cpuMemory = 0;

	for(const ClassAllocator& alloc : m_callocs)
	{
		if(alloc.m_isDeviceMemory)
		{
			gpuMemory += alloc.getAllocatedMemory();
		}
		else
		{
			cpuMemory += alloc.getAllocatedMemory();
		}
	}
}

} // end namespace anki
