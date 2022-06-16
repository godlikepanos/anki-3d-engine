// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/CommandBufferFactory.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

static VulkanQueueType getQueueTypeFromCommandBufferFlags(CommandBufferFlag flags,
														  const VulkanQueueFamilies& queueFamilies)
{
	ANKI_ASSERT(!!(flags & CommandBufferFlag::GENERAL_WORK) ^ !!(flags & CommandBufferFlag::COMPUTE_WORK));
	if(!(flags & CommandBufferFlag::GENERAL_WORK) && queueFamilies[VulkanQueueType::COMPUTE] != MAX_U32)
	{
		return VulkanQueueType::COMPUTE;
	}
	else
	{
		ANKI_ASSERT(queueFamilies[VulkanQueueType::GENERAL] != MAX_U32);
		return VulkanQueueType::GENERAL;
	}
}

MicroCommandBuffer::~MicroCommandBuffer()
{
	reset();

	if(m_handle)
	{
		vkFreeCommandBuffers(m_threadAlloc->m_factory->m_dev, m_threadAlloc->m_pools[m_queue], 1, &m_handle);
		m_handle = {};

		[[maybe_unused]] const U32 count = m_threadAlloc->m_factory->m_createdCmdBufferCount.fetchSub(1);
		ANKI_ASSERT(count > 0);
	}
}

void MicroCommandBuffer::reset()
{
	ANKI_TRACE_SCOPED_EVENT(VK_COMMAND_BUFFER_RESET);

	ANKI_ASSERT(m_refcount.load() == 0);
	ANKI_ASSERT(!m_fence.isCreated());

	for(GrObjectType type : EnumIterable<GrObjectType>())
	{
		m_objectRefs[type].destroy(m_fastAlloc);
	}

	m_fastAlloc.getMemoryPool().reset();
}

Error CommandBufferThreadAllocator::init()
{
	for(VulkanQueueType qtype : EnumIterable<VulkanQueueType>())
	{
		if(m_factory->m_queueFamilies[qtype] == MAX_U32)
		{
			continue;
		}

		VkCommandPoolCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		ci.queueFamilyIndex = m_factory->m_queueFamilies[qtype];

		ANKI_VK_CHECK(vkCreateCommandPool(m_factory->m_dev, &ci, nullptr, &m_pools[qtype]));
	}

	for(U32 secondLevel = 0; secondLevel < 2; ++secondLevel)
	{
		for(U32 smallBatch = 0; smallBatch < 2; ++smallBatch)
		{
			for(VulkanQueueType queue : EnumIterable<VulkanQueueType>())
			{
				MicroObjectRecycler<MicroCommandBuffer>& recycler = m_recyclers[secondLevel][smallBatch][queue];

				recycler.init(m_factory->m_alloc);
			}
		}
	}

	return Error::NONE;
}

void CommandBufferThreadAllocator::destroy()
{
	for(U32 secondLevel = 0; secondLevel < 2; ++secondLevel)
	{
		for(U32 smallBatch = 0; smallBatch < 2; ++smallBatch)
		{
			for(VulkanQueueType queue : EnumIterable<VulkanQueueType>())
			{
				m_recyclers[secondLevel][smallBatch][queue].destroy();
			}
		}
	}

	for(VkCommandPool& pool : m_pools)
	{
		if(pool)
		{
			vkDestroyCommandPool(m_factory->m_dev, pool, nullptr);
			pool = VK_NULL_HANDLE;
		}
	}
}

Error CommandBufferThreadAllocator::newCommandBuffer(CommandBufferFlag cmdbFlags, MicroCommandBufferPtr& outPtr)
{
	ANKI_ASSERT(!!(cmdbFlags & CommandBufferFlag::COMPUTE_WORK) ^ !!(cmdbFlags & CommandBufferFlag::GENERAL_WORK));

	const Bool secondLevel = !!(cmdbFlags & CommandBufferFlag::SECOND_LEVEL);
	const Bool smallBatch = !!(cmdbFlags & CommandBufferFlag::SMALL_BATCH);
	const VulkanQueueType queue = getQueueTypeFromCommandBufferFlags(cmdbFlags, m_factory->m_queueFamilies);

	MicroObjectRecycler<MicroCommandBuffer>& recycler = m_recyclers[secondLevel][smallBatch][queue];

	MicroCommandBuffer* out = recycler.findToReuse();

	if(ANKI_UNLIKELY(out == nullptr))
	{
		// Create a new one

		VkCommandBufferAllocateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		ci.commandPool = m_pools[queue];
		ci.level = (secondLevel) ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		ci.commandBufferCount = 1;

		ANKI_TRACE_INC_COUNTER(VK_COMMAND_BUFFER_CREATE, 1);
		VkCommandBuffer cmdb;
		ANKI_VK_CHECK(vkAllocateCommandBuffers(m_factory->m_dev, &ci, &cmdb));

		MicroCommandBuffer* newCmdb = getAllocator().newInstance<MicroCommandBuffer>(this);

		newCmdb->m_fastAlloc =
			StackAllocator<U8>(m_factory->m_alloc.getMemoryPool().getAllocationCallback(),
							   m_factory->m_alloc.getMemoryPool().getAllocationCallbackUserData(), 256_KB, 2.0f);

		newCmdb->m_handle = cmdb;
		newCmdb->m_flags = cmdbFlags;
		newCmdb->m_queue = queue;

		out = newCmdb;

		m_factory->m_createdCmdBufferCount.fetchAdd(1);
	}
	else
	{
		for([[maybe_unused]] GrObjectType type : EnumIterable<GrObjectType>())
		{
			ANKI_ASSERT(out->m_objectRefs[type].getSize() == 0);
		}
	}

	ANKI_ASSERT(out && out->m_refcount.load() == 0);
	ANKI_ASSERT(out->m_flags == cmdbFlags);
	outPtr.reset(out);
	return Error::NONE;
}

void CommandBufferThreadAllocator::deleteCommandBuffer(MicroCommandBuffer* ptr)
{
	ANKI_ASSERT(ptr);

	const Bool secondLevel = !!(ptr->m_flags & CommandBufferFlag::SECOND_LEVEL);
	const Bool smallBatch = !!(ptr->m_flags & CommandBufferFlag::SMALL_BATCH);

	m_recyclers[secondLevel][smallBatch][ptr->m_queue].recycle(ptr);
}

Error CommandBufferFactory::init(GrAllocator<U8> alloc, VkDevice dev, const VulkanQueueFamilies& queueFamilies)
{
	ANKI_ASSERT(dev);

	m_alloc = alloc;
	m_dev = dev;
	m_queueFamilies = queueFamilies;
	return Error::NONE;
}

void CommandBufferFactory::destroy()
{
	// First trim the caches for all recyclers. This will release the primaries and populate the recyclers of
	// secondaries
	for(CommandBufferThreadAllocator* talloc : m_threadAllocs)
	{
		for(U32 secondLevel = 0; secondLevel < 2; ++secondLevel)
		{
			for(U32 smallBatch = 0; smallBatch < 2; ++smallBatch)
			{
				for(VulkanQueueType queue : EnumIterable<VulkanQueueType>())
				{
					talloc->m_recyclers[secondLevel][smallBatch][queue].trimCache();
				}
			}
		}
	}

	for(CommandBufferThreadAllocator* talloc : m_threadAllocs)
	{
		talloc->destroy();
		m_alloc.deleteInstance(talloc);
	}

	m_threadAllocs.destroy(m_alloc);
}

Error CommandBufferFactory::newCommandBuffer(ThreadId tid, CommandBufferFlag cmdbFlags, MicroCommandBufferPtr& ptr)
{
	CommandBufferThreadAllocator* alloc = nullptr;

	// Get the thread allocator
	{
		class Comp
		{
		public:
			Bool operator()(const CommandBufferThreadAllocator* a, ThreadId tid) const
			{
				return a->m_tid < tid;
			}

			Bool operator()(ThreadId tid, const CommandBufferThreadAllocator* a) const
			{
				return tid < a->m_tid;
			}
		};

		// Find using binary search
		{
			RLockGuard<RWMutex> lock(m_threadAllocMtx);
			auto it = binarySearch(m_threadAllocs.getBegin(), m_threadAllocs.getEnd(), tid, Comp());
			alloc = (it != m_threadAllocs.getEnd()) ? (*it) : nullptr;
		}

		if(ANKI_UNLIKELY(alloc == nullptr))
		{
			WLockGuard<RWMutex> lock(m_threadAllocMtx);

			// Check again
			auto it = binarySearch(m_threadAllocs.getBegin(), m_threadAllocs.getEnd(), tid, Comp());
			alloc = (it != m_threadAllocs.getEnd()) ? (*it) : nullptr;

			if(alloc == nullptr)
			{
				alloc = m_alloc.newInstance<CommandBufferThreadAllocator>(this, tid);

				m_threadAllocs.resize(m_alloc, m_threadAllocs.getSize() + 1);
				m_threadAllocs[m_threadAllocs.getSize() - 1] = alloc;

				// Sort for fast find
				std::sort(m_threadAllocs.getBegin(), m_threadAllocs.getEnd(),
						  [](const CommandBufferThreadAllocator* a, const CommandBufferThreadAllocator* b) {
							  return a->m_tid < b->m_tid;
						  });

				ANKI_CHECK(alloc->init());
			}
		}
	}

	ANKI_ASSERT(alloc);
	ANKI_ASSERT(alloc->m_tid == tid);
	ANKI_CHECK(alloc->newCommandBuffer(cmdbFlags, ptr));

	return Error::NONE;
}

} // end namespace anki
