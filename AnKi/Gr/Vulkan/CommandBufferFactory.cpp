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

void MicroCommandBuffer::destroy()
{
	reset();

	if(m_handle)
	{
		vkFreeCommandBuffers(m_threadAlloc->m_factory->m_dev, m_threadAlloc->m_pools[m_queue], 1, &m_handle);
		m_handle = {};
	}
}

void MicroCommandBuffer::reset()
{
	ANKI_TRACE_SCOPED_EVENT(VK_COMMAND_BUFFER_RESET);

	ANKI_ASSERT(m_refcount.load() == 0);
	ANKI_ASSERT(!m_fence.isCreated() || m_fence->done());

	for(GrObjectType type : EnumIterable<GrObjectType>())
	{
		m_objectRefs[type].destroy(m_fastAlloc);
	}

	m_fastAlloc.getMemoryPool().reset();

	m_fence = {};
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

	return Error::NONE;
}

void CommandBufferThreadAllocator::destroyList(IntrusiveList<MicroCommandBuffer>& list)
{
	while(!list.isEmpty())
	{
		MicroCommandBuffer* ptr = list.popFront();
		ptr->destroy();
		getAllocator().deleteInstance(ptr);
#if ANKI_EXTRA_CHECKS
		m_createdCmdbs.fetchSub(1);
#endif
	}
}

void CommandBufferThreadAllocator::destroyLists()
{
	for(U i = 0; i < 2; ++i)
	{
		for(U j = 0; j < 2; ++j)
		{
			for(VulkanQueueType qtype : EnumIterable<VulkanQueueType>())
			{
				CmdbType& type = m_types[i][j][qtype];

				destroyList(type.m_deletedCmdbs);
				destroyList(type.m_readyCmdbs);
				destroyList(type.m_inUseCmdbs);
			}
		}
	}
}

void CommandBufferThreadAllocator::destroy()
{
	for(VkCommandPool& pool : m_pools)
	{
		if(pool)
		{
			vkDestroyCommandPool(m_factory->m_dev, pool, nullptr);
			pool = VK_NULL_HANDLE;
		}
	}

	ANKI_ASSERT(m_createdCmdbs.load() == 0 && "Someone still holds references to command buffers");
}

Error CommandBufferThreadAllocator::newCommandBuffer(CommandBufferFlag cmdbFlags, MicroCommandBufferPtr& outPtr,
													 Bool& createdNew)
{
	ANKI_ASSERT(!!(cmdbFlags & CommandBufferFlag::COMPUTE_WORK) ^ !!(cmdbFlags & CommandBufferFlag::GENERAL_WORK));
	createdNew = false;

	const Bool secondLevel = !!(cmdbFlags & CommandBufferFlag::SECOND_LEVEL);
	const Bool smallBatch = !!(cmdbFlags & CommandBufferFlag::SMALL_BATCH);
	const VulkanQueueType queue = getQueueTypeFromCommandBufferFlags(cmdbFlags, m_factory->m_queueFamilies);

	CmdbType& type = m_types[secondLevel][smallBatch][queue];

	// Move the deleted to (possibly) in-use or ready
	{
		LockGuard<Mutex> lock(type.m_deletedMtx);

		while(!type.m_deletedCmdbs.isEmpty())
		{
			MicroCommandBuffer* ptr = type.m_deletedCmdbs.popFront();

			if(secondLevel)
			{
				type.m_readyCmdbs.pushFront(ptr);
				ptr->reset();
			}
			else
			{
				type.m_inUseCmdbs.pushFront(ptr);
			}
		}
	}

	// Reset the in-use command buffers and try to get one available
	MicroCommandBuffer* out = nullptr;
	if(!secondLevel)
	{
		// Primary

		// Try to reuse a ready buffer
		if(!type.m_readyCmdbs.isEmpty())
		{
			out = type.m_readyCmdbs.popFront();
		}

		// Do a sweep and move in-use buffers to ready
		IntrusiveList<MicroCommandBuffer> inUseCmdbs; // Push to temporary because we are iterating
		while(!type.m_inUseCmdbs.isEmpty())
		{
			MicroCommandBuffer* inUseCmdb = type.m_inUseCmdbs.popFront();

			if(!inUseCmdb->m_fence.isCreated() || inUseCmdb->m_fence->done())
			{
				// It's ready

				if(out)
				{
					type.m_readyCmdbs.pushFront(inUseCmdb);
					inUseCmdb->reset();
				}
				else
				{
					out = inUseCmdb;
				}
			}
			else
			{
				inUseCmdbs.pushBack(inUseCmdb);
			}
		}

		ANKI_ASSERT(type.m_inUseCmdbs.isEmpty());
		type.m_inUseCmdbs = std::move(inUseCmdbs);
	}
	else
	{
		// Secondary

		ANKI_ASSERT(type.m_inUseCmdbs.isEmpty());

		if(!type.m_readyCmdbs.isEmpty())
		{
			out = type.m_readyCmdbs.popFront();
		}
	}

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

#if ANKI_EXTRA_CHECKS
		m_createdCmdbs.fetchAdd(1);
#endif

		newCmdb->m_fastAlloc =
			StackAllocator<U8>(m_factory->m_alloc.getMemoryPool().getAllocationCallback(),
							   m_factory->m_alloc.getMemoryPool().getAllocationCallbackUserData(), 256_KB, 2.0f);

		newCmdb->m_handle = cmdb;
		newCmdb->m_flags = cmdbFlags;
		newCmdb->m_queue = queue;

		out = newCmdb;

		createdNew = true;
	}
	else
	{
		out->reset();
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

	CmdbType& type = m_types[secondLevel][smallBatch][ptr->m_queue];

	LockGuard<Mutex> lock(type.m_deletedMtx);
	type.m_deletedCmdbs.pushBack(ptr);
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
	// Run 2 times because destroyLists() populates other allocators' lists
	for(U i = 0; i < 2; ++i)
	{
		for(CommandBufferThreadAllocator* alloc : m_threadAllocs)
		{
			alloc->destroyLists();
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
	Bool createdNew;
	ANKI_CHECK(alloc->newCommandBuffer(cmdbFlags, ptr, createdNew));
	if(createdNew)
	{
		m_createdCmdBufferCount.fetchAdd(1);
	}

	return Error::NONE;
}

} // end namespace anki
