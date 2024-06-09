// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/VkCommandBufferFactory.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Core/StatsSet.h>

namespace anki {

static StatCounter g_commandBufferCountStatVar(StatCategory::kMisc, "CommandBufferCount", StatFlag::kNone);

static GpuQueueType getQueueTypeFromCommandBufferFlags(CommandBufferFlag flags, const VulkanQueueFamilies& queueFamilies)
{
	ANKI_ASSERT(!!(flags & CommandBufferFlag::kGeneralWork) ^ !!(flags & CommandBufferFlag::kComputeWork));
	if(!(flags & CommandBufferFlag::kGeneralWork) && queueFamilies[GpuQueueType::kCompute] != kMaxU32)
	{
		return GpuQueueType::kCompute;
	}
	else
	{
		ANKI_ASSERT(queueFamilies[GpuQueueType::kGeneral] != kMaxU32);
		return GpuQueueType::kGeneral;
	}
}

void MicroCommandBufferPtrDeleter::operator()(MicroCommandBuffer* ptr)
{
	ANKI_ASSERT(ptr);
	ptr->m_threadAlloc->deleteCommandBuffer(ptr);
}

MicroCommandBuffer::~MicroCommandBuffer()
{
	reset();

	m_dsAllocator.destroy();

	if(m_handle)
	{
		vkFreeCommandBuffers(getVkDevice(), m_threadAlloc->m_pools[m_queue], 1, &m_handle);
		m_handle = {};

		g_commandBufferCountStatVar.decrement(1_U64);
	}
}

void MicroCommandBuffer::reset()
{
	ANKI_TRACE_SCOPED_EVENT(VkCommandBufferReset);

	ANKI_ASSERT(m_refcount.load() == 0);
	ANKI_ASSERT(!m_fence.isCreated());

	for(GrObjectType type : EnumIterable<GrObjectType>())
	{
		m_objectRefs[type].destroy();
	}

	m_dsAllocator.reset();

	m_fastPool.reset();
}

Error CommandBufferThreadAllocator::init()
{
	for(GpuQueueType qtype : EnumIterable<GpuQueueType>())
	{
		if(CommandBufferFactory::getSingleton().m_queueFamilies[qtype] == kMaxU32)
		{
			continue;
		}

		VkCommandPoolCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		ci.queueFamilyIndex = CommandBufferFactory::getSingleton().m_queueFamilies[qtype];

		ANKI_VK_CHECK(vkCreateCommandPool(getVkDevice(), &ci, nullptr, &m_pools[qtype]));
	}

	return Error::kNone;
}

void CommandBufferThreadAllocator::destroy()
{
	for(U32 smallBatch = 0; smallBatch < 2; ++smallBatch)
	{
		for(GpuQueueType queue : EnumIterable<GpuQueueType>())
		{
			m_recyclers[smallBatch][queue].destroy();
		}
	}

	for(VkCommandPool& pool : m_pools)
	{
		if(pool)
		{
			vkDestroyCommandPool(getVkDevice(), pool, nullptr);
			pool = VK_NULL_HANDLE;
		}
	}
}

Error CommandBufferThreadAllocator::newCommandBuffer(CommandBufferFlag cmdbFlags, MicroCommandBufferPtr& outPtr)
{
	ANKI_ASSERT(!!(cmdbFlags & CommandBufferFlag::kComputeWork) ^ !!(cmdbFlags & CommandBufferFlag::kGeneralWork));

	const Bool smallBatch = !!(cmdbFlags & CommandBufferFlag::kSmallBatch);
	const GpuQueueType queue = getQueueTypeFromCommandBufferFlags(cmdbFlags, CommandBufferFactory::getSingleton().m_queueFamilies);

	MicroObjectRecycler<MicroCommandBuffer>& recycler = m_recyclers[smallBatch][queue];

	MicroCommandBuffer* out = recycler.findToReuse();

	if(out == nullptr) [[unlikely]]
	{
		// Create a new one

		VkCommandBufferAllocateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		ci.commandPool = m_pools[queue];
		ci.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		ci.commandBufferCount = 1;

		ANKI_TRACE_INC_COUNTER(VkCommandBufferCreate, 1);
		g_commandBufferCountStatVar.increment(1_U64);
		VkCommandBuffer cmdb;
		ANKI_VK_CHECK(vkAllocateCommandBuffers(getVkDevice(), &ci, &cmdb));

		MicroCommandBuffer* newCmdb = newInstance<MicroCommandBuffer>(GrMemoryPool::getSingleton(), this);

		newCmdb->m_fastPool.init(GrMemoryPool::getSingleton().getAllocationCallback(), GrMemoryPool::getSingleton().getAllocationCallbackUserData(),
								 256_KB, 2.0f);

		for(DynamicArray<GrObjectPtr, MemoryPoolPtrWrapper<StackMemoryPool>>& arr : newCmdb->m_objectRefs)
		{
			arr = DynamicArray<GrObjectPtr, MemoryPoolPtrWrapper<StackMemoryPool>>(&newCmdb->m_fastPool);
		}

		newCmdb->m_handle = cmdb;
		newCmdb->m_flags = cmdbFlags;
		newCmdb->m_queue = queue;

		out = newCmdb;
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
	return Error::kNone;
}

void CommandBufferThreadAllocator::deleteCommandBuffer(MicroCommandBuffer* ptr)
{
	ANKI_ASSERT(ptr);

	const Bool smallBatch = !!(ptr->m_flags & CommandBufferFlag::kSmallBatch);

	m_recyclers[smallBatch][ptr->m_queue].recycle(ptr);
}

void CommandBufferFactory::destroy()
{
	// First trim the caches for all recyclers.
	for(CommandBufferThreadAllocator* talloc : m_threadAllocs)
	{
		for(U32 smallBatch = 0; smallBatch < 2; ++smallBatch)
		{
			for(GpuQueueType queue : EnumIterable<GpuQueueType>())
			{
				talloc->m_recyclers[smallBatch][queue].trimCache();
			}
		}
	}

	for(CommandBufferThreadAllocator* talloc : m_threadAllocs)
	{
		talloc->destroy();
		deleteInstance(GrMemoryPool::getSingleton(), talloc);
	}

	m_threadAllocs.destroy();
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

		if(alloc == nullptr) [[unlikely]]
		{
			WLockGuard<RWMutex> lock(m_threadAllocMtx);

			// Check again
			auto it = binarySearch(m_threadAllocs.getBegin(), m_threadAllocs.getEnd(), tid, Comp());
			alloc = (it != m_threadAllocs.getEnd()) ? (*it) : nullptr;

			if(alloc == nullptr)
			{
				alloc = newInstance<CommandBufferThreadAllocator>(GrMemoryPool::getSingleton(), tid);

				m_threadAllocs.resize(m_threadAllocs.getSize() + 1);
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

	return Error::kNone;
}

} // end namespace anki
