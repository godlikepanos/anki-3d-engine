// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
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

static unsigned long long int operator"" _B(unsigned long long int x)
{
	return x;
}

static unsigned long long int operator"" _KB(unsigned long long int x)
{
	return x * 1024;
}

static unsigned long long int operator"" _MB(unsigned long long int x)
{
	return x * 1024 * 1024;
}

static const Array<ClassInf, CLASS_COUNT> CLASSES = {{{256_B, 16_KB},
	{4_KB, 256_KB},
	{128_KB, 8_MB},
	{1_MB, 64_MB},
	{16_MB, 128_MB},
	{64_MB, 256_MB},
	{128_MB, 256_MB}}};

class GpuMemoryManager::Memory : public ClassAllocatorMemory, public IntrusiveListEnabled<GpuMemoryManager::Memory>
{
public:
	VkDeviceMemory m_handle = VK_NULL_HANDLE;
	U8 m_classIdx = MAX_U8;
};

class GpuMemoryManager::Interface : public ClassAllocatorInterface
{
public:
	uint32_t m_memTypeIdx = MAX_U32;
	MemoryTypeCommon* m_memTypeCommon = nullptr;
	GrAllocator<U8> m_alloc;
	VkDevice m_dev = VK_NULL_HANDLE;

	Interface() = default;

	Error allocate(U classIdx, ClassAllocatorMemory*& mem);

	void free(ClassAllocatorMemory* mem);

	U getClassCount() const
	{
		return CLASS_COUNT;
	}

	void getClassInfo(U classIdx, PtrSize& slotSize, PtrSize& chunkSize) const
	{
		slotSize = CLASSES[classIdx].m_slotSize;
		chunkSize = CLASSES[classIdx].m_chunkSize;
	}
};

class GpuMemoryManager::MemoryTypeCommon
{
public:
	Array<IntrusiveList<Memory>, CLASS_COUNT> m_vacantMemory;
	Mutex m_mtx;
};

Error GpuMemoryManager::Interface::allocate(U classIdx, ClassAllocatorMemory*& mem)
{
	const PtrSize chunkSize = CLASSES[classIdx].m_chunkSize;

	LockGuard<Mutex> lock(m_memTypeCommon->m_mtx);

	Memory* pmem = nullptr;

	if(!m_memTypeCommon->m_vacantMemory[classIdx].isEmpty())
	{
		pmem = &m_memTypeCommon->m_vacantMemory[classIdx].getFront();
		m_memTypeCommon->m_vacantMemory[classIdx].popFront();
	}
	else
	{
		pmem = m_alloc.newInstance<Memory>();

		VkMemoryAllocateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		ci.allocationSize = chunkSize;
		ci.memoryTypeIndex = m_memTypeIdx;
		ANKI_VK_CHECKF(vkAllocateMemory(m_dev, &ci, nullptr, &pmem->m_handle));

		pmem->m_classIdx = classIdx;
	}

	ANKI_ASSERT(pmem);
	mem = pmem;
	return ErrorCode::NONE;
}

void GpuMemoryManager::Interface::free(ClassAllocatorMemory* mem)
{
	ANKI_ASSERT(mem);
	Memory* pmem = static_cast<Memory*>(mem);
	ANKI_ASSERT(pmem->m_handle);

	LockGuard<Mutex> lock(m_memTypeCommon->m_mtx);
	m_memTypeCommon->m_vacantMemory[pmem->m_classIdx].pushBack(pmem);
}

void GpuMemoryManager::init(VkPhysicalDevice pdev, VkDevice dev, GrAllocator<U8> alloc)
{
}

} // end namespace anki
