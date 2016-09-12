// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/GpuMemoryAllocator.h>
#include <anki/util/BitSet.h>

namespace anki
{

/// Define this type just to show what is what.
using Page = U32;

/// The size of the page. This is the minumum allocation size as well.
const U PAGE_SIZE = 8 * 1024;

/// Max number of sub allocations (aka slots) per chunk.
const U MAX_SLOTS_PER_CHUNK = 128;

#define ANKI_CHECK_HANDLE(handle_) ANKI_ASSERT(handle_.m_memory&& handle_.m_chunk&& handle_.m_chunk->m_class)

class GpuMemoryAllocatorChunk : public IntrusiveListEnabled<GpuMemoryAllocatorChunk>
{
public:
	/// GPU mem.
	VkDeviceMemory m_mem = VK_NULL_HANDLE;

	/// The in use slots mask.
	BitSet<MAX_SLOTS_PER_CHUNK, U8> m_inUseSlots = {false};

	/// The number of in-use slots.
	U32 m_inUseSlotCount = 0;

	/// The owner.
	GpuMemoryAllocatorClass* m_class = nullptr;

	/// It points to a CPU address if mapped.
	U8* m_mappedAddress = nullptr;

	/// Protect the m_mappedAddress. It's a SpinLock because we don't want a whole mutex for every
	/// GpuMemoryAllocatorChunk.
	SpinLock m_mtx;

	/// If true it contains linear resources.
	Bool8 m_linearResources = false;
};

class GpuMemoryAllocatorClass
{
public:
	/// The active chunks.
	IntrusiveList<GpuMemoryAllocatorChunk> m_inUseChunks;

	/// The empty chunks.
	IntrusiveList<GpuMemoryAllocatorChunk> m_unusedChunks;

	/// The size of each chunk.
	Page m_chunkPages = 0;

	/// The max slot size for this class.
	U32 m_maxSlotSize = 0;

	/// The number of slots for a single chunk.
	U32 m_slotsPerChunkCount = 0;

	Mutex m_mtx;
};

GpuMemoryAllocatorMemType::~GpuMemoryAllocatorMemType()
{
	for(Class& cl : m_classes)
	{
		if(!cl.m_inUseChunks.isEmpty())
		{
			ANKI_LOGW("Forgot to deallocate GPU memory");

			while(!cl.m_inUseChunks.isEmpty())
			{
				Chunk* chunk = &cl.m_inUseChunks.getBack();
				cl.m_inUseChunks.popBack();

				// Unmap
				if(chunk->m_mappedAddress)
				{
					vkUnmapMemory(m_dev, chunk->m_mem);
				}

				vkFreeMemory(m_dev, chunk->m_mem, nullptr);
				m_alloc.deleteInstance(chunk);
			}
		}

		while(!cl.m_unusedChunks.isEmpty())
		{
			Chunk* chunk = &cl.m_unusedChunks.getBack();
			cl.m_unusedChunks.popBack();

			// Unmap
			if(chunk->m_mappedAddress)
			{
				vkUnmapMemory(m_dev, chunk->m_mem);
			}

			vkFreeMemory(m_dev, chunk->m_mem, nullptr);
			m_alloc.deleteInstance(chunk);
		}
	}

	m_classes.destroy(m_alloc);
}

void GpuMemoryAllocatorMemType::init(GenericMemoryPoolAllocator<U8> alloc, VkDevice dev, U memoryTypeIdx)
{
	m_alloc = alloc;
	m_dev = dev;
	m_memIdx = memoryTypeIdx;

	//
	// Initialize the classes
	//
	const U CLASS_COUNT = 6;
	m_classes.create(m_alloc, CLASS_COUNT);
	static const CString MESSAGE = "VK: Creating memory class. "
								   "Chunk size: %u, maxSlotSize: %u, allocsPerChunk: %u";

#define ANKI_PRINT_MSG()                                                                                               \
	if(memoryTypeIdx == 0)                                                                                             \
	{                                                                                                                  \
		ANKI_LOGI(                                                                                                     \
			&MESSAGE[0], c.m_chunkPages* PAGE_SIZE, c.m_maxSlotSize, c.m_chunkPages* PAGE_SIZE / c.m_maxSlotSize)      \
	}

	// 1st class. From (0, 256] bytes
	{
		Class& c = m_classes[0];
		c.m_chunkPages = 2;
		c.m_maxSlotSize = 256;
		ANKI_PRINT_MSG();
	}

	// 2st class. From (256, 4K] bytes
	{
		Class& c = m_classes[1];
		c.m_chunkPages = 32;
		c.m_maxSlotSize = 4 * 1024;
		ANKI_PRINT_MSG();
	}

	// 3rd class. From (4K, 128K] bytes
	{
		Class& c = m_classes[2];
		c.m_chunkPages = 1024;
		c.m_maxSlotSize = 128 * 1024;
		ANKI_PRINT_MSG();
	}

	// 4rth class. From (128K, 1M] bytes
	{
		Class& c = m_classes[3];
		c.m_chunkPages = 4 * 1024;
		c.m_maxSlotSize = 1 * 1024 * 1024;
		ANKI_PRINT_MSG();
	}

	// 5th class. From (1M, 10M] bytes
	{
		Class& c = m_classes[4];
		c.m_chunkPages = 10 * 1024;
		c.m_maxSlotSize = 10 * 1024 * 1024;
		ANKI_PRINT_MSG();
	}

	// 6th class. From (10M, 80M] bytes
	{
		Class& c = m_classes[5];
		c.m_chunkPages = 20 * 1024;
		c.m_maxSlotSize = 80 * 1024 * 1024;
		ANKI_PRINT_MSG();
	}

	for(Class& c : m_classes)
	{
		ANKI_ASSERT(((c.m_chunkPages * PAGE_SIZE) % c.m_maxSlotSize) == 0);
		c.m_slotsPerChunkCount = (c.m_chunkPages * PAGE_SIZE) / c.m_maxSlotSize;
		ANKI_ASSERT(c.m_slotsPerChunkCount <= MAX_SLOTS_PER_CHUNK);
	}
}

GpuMemoryAllocatorMemType::Class* GpuMemoryAllocatorMemType::findClass(PtrSize size, U32 alignment)
{
	ANKI_ASSERT(size > 0 && alignment > 0);

	PtrSize lowLimit = 0;
	Class* it = m_classes.getBegin();
	const Class* end = m_classes.getEnd();

	while(it != end)
	{
		PtrSize highLimit = it->m_maxSlotSize;

		if(size > lowLimit && size <= highLimit)
		{
			if(alignment <= highLimit)
			{
				// Found the class
				return it;
			}
			else
			{
				// The class found doesn't have the proper alignment. Need to
				// go higher

				while(++it != end)
				{
					if(alignment <= it->m_maxSlotSize)
					{
						// Now found something
						return it;
					}
				}
			}
		}

		lowLimit = highLimit;
		++it;
	}

	return nullptr;
}

GpuMemoryAllocatorMemType::Chunk* GpuMemoryAllocatorMemType::findChunkWithUnusedSlot(Class& cl, Bool linearResource)
{
	auto it = cl.m_inUseChunks.getBegin();
	const auto end = cl.m_inUseChunks.getEnd();
	while(it != end)
	{
		if(it->m_inUseSlotCount < cl.m_slotsPerChunkCount && it->m_linearResources == linearResource)
		{
			return &(*it);
		}

		++it;
	}

	return nullptr;
}

GpuMemoryAllocatorMemType::Chunk& GpuMemoryAllocatorMemType::createChunk(Class& cl, Bool linearResources)
{
	Chunk* chunk = nullptr;

	if(cl.m_unusedChunks.isEmpty())
	{
		// Create new

		chunk = m_alloc.newInstance<Chunk>();
		chunk->m_class = &cl;

		VkMemoryAllocateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		ci.allocationSize = cl.m_chunkPages * PAGE_SIZE;
		ci.memoryTypeIndex = m_memIdx;
		ANKI_VK_CHECKF(vkAllocateMemory(m_dev, &ci, nullptr, &chunk->m_mem));
	}
	else
	{
		// Recycle

		chunk = &cl.m_unusedChunks.getFront();
		cl.m_unusedChunks.popFront();
	}

	chunk->m_linearResources = linearResources;
	cl.m_inUseChunks.pushBack(chunk);

	ANKI_ASSERT(chunk->m_mem && chunk->m_class == &cl && chunk->m_inUseSlotCount == 0 && !chunk->m_inUseSlots.getAny());

	return *chunk;
}

void GpuMemoryAllocatorMemType::allocate(
	PtrSize size, U alignment, Bool linearResource, GpuMemoryAllocationHandle& handle)
{
	handle.m_memory = VK_NULL_HANDLE;

	// Find the class for the given size
	Class* cl = findClass(size, alignment);
	ANKI_ASSERT(cl && "Didn't found a suitable class");

	LockGuard<Mutex> lock(cl->m_mtx);
	Chunk* chunk = findChunkWithUnusedSlot(*cl, linearResource);

	// Create a new chunk if needed
	if(chunk == nullptr)
	{
		chunk = &createChunk(*cl, linearResource);
	}

	// Allocate from chunk
	U bitCount = cl->m_slotsPerChunkCount;
	for(U i = 0; i < bitCount; ++i)
	{
		if(!chunk->m_inUseSlots.get(i))
		{
			// Found an empty slot, allocate from it
			chunk->m_inUseSlots.set(i);
			++chunk->m_inUseSlotCount;

			handle.m_memory = chunk->m_mem;
			handle.m_offset = i * cl->m_maxSlotSize;
			handle.m_chunk = chunk;

			break;
		}
	}

	ANKI_ASSERT(handle.m_memory && handle.m_chunk);
	ANKI_ASSERT(isAligned(alignment, handle.m_offset));
}

void GpuMemoryAllocatorMemType::destroyChunk(Class& cl, Chunk& chunk)
{
	// Push the chunk to unused area
	cl.m_inUseChunks.erase(&chunk);
	cl.m_unusedChunks.pushBack(&chunk);

	// Unmap. This may free some VA
	if(chunk.m_mappedAddress)
	{
		vkUnmapMemory(m_dev, chunk.m_mem);
		chunk.m_mappedAddress = nullptr;
	}
}

void GpuMemoryAllocatorMemType::free(GpuMemoryAllocationHandle& handle)
{
	ANKI_CHECK_HANDLE(handle);

	Chunk& chunk = *handle.m_chunk;
	Class& cl = *chunk.m_class;

	LockGuard<Mutex> lock(cl.m_mtx);
	U slotIdx = handle.m_offset / cl.m_maxSlotSize;

	ANKI_ASSERT(chunk.m_inUseSlots.get(slotIdx));
	ANKI_ASSERT(chunk.m_inUseSlotCount > 0);
	chunk.m_inUseSlots.unset(slotIdx);
	--chunk.m_inUseSlotCount;

	if(chunk.m_inUseSlotCount == 0)
	{
		destroyChunk(cl, chunk);
	}

	handle = {};
}

void* GpuMemoryAllocatorMemType::getMappedAddress(GpuMemoryAllocationHandle& handle)
{
	ANKI_CHECK_HANDLE(handle);

	Chunk& chunk = *handle.m_chunk;
	U8* out = nullptr;

	{
		LockGuard<SpinLock> lock(chunk.m_mtx);
		if(chunk.m_mappedAddress)
		{
			out = chunk.m_mappedAddress;
		}
		else
		{
			ANKI_VK_CHECKF(vkMapMemory(
				m_dev, chunk.m_mem, 0, chunk.m_class->m_chunkPages * PAGE_SIZE, 0, reinterpret_cast<void**>(&out)));

			chunk.m_mappedAddress = out;
		}
	}

	ANKI_ASSERT(out);
	return static_cast<void*>(out + handle.m_offset);
}

GpuMemoryAllocator::~GpuMemoryAllocator()
{
}

void GpuMemoryAllocator::init(VkPhysicalDevice pdev, VkDevice dev, GrAllocator<U8> alloc)
{
	vkGetPhysicalDeviceMemoryProperties(pdev, &m_memoryProperties);

	// Create the high level allocators
	m_gpuAllocs.create(alloc, m_memoryProperties.memoryTypeCount);
	U idx = 0;
	for(GpuMemoryAllocatorMemType& a : m_gpuAllocs)
	{
		a.init(alloc, dev, idx++);
	}

	m_dev = dev;
	m_alloc = alloc;
}

void GpuMemoryAllocator::destroy()
{
	m_gpuAllocs.destroy(m_alloc);
}

U GpuMemoryAllocator::findMemoryType(
	U resourceMemTypeBits, VkMemoryPropertyFlags preferFlags, VkMemoryPropertyFlags avoidFlags) const
{
	U preferedHigh = MAX_U32;
	U preferedMed = MAX_U32;

	// Iterate all mem types
	for(U i = 0; i < m_memoryProperties.memoryTypeCount; i++)
	{
		if(resourceMemTypeBits & (1u << i))
		{
			VkMemoryPropertyFlags flags = m_memoryProperties.memoryTypes[i].propertyFlags;

			if((flags & preferFlags) == preferFlags)
			{
				preferedMed = i;

				if((flags & avoidFlags) != avoidFlags)
				{
					preferedHigh = i;
				}
			}
		}
	}

	if(preferedHigh < MAX_U32)
	{
		return preferedHigh;
	}
	else
	{
		return preferedMed;
	}
}

} // end namespace anki
