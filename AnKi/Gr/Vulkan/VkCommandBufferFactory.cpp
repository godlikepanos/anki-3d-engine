// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/VkCommandBufferFactory.h>
#include <AnKi/Gr/Vulkan/VkGrManager.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Core/StatsSet.h>

namespace anki {

static StatCounter g_commandBufferCountStatVar(StatCategory::kMisc, "CommandBufferCount", StatFlag::kNone);

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
		const U32 queueFamilyIdx =
			(m_queue == GpuQueueType::kCompute && getGrManagerImpl().getAsyncComputeType() == AsyncComputeType::kLowPriorityQueue) ? 0 : U32(m_queue);

		vkFreeCommandBuffers(getVkDevice(), m_threadAlloc->m_pools[queueFamilyIdx], 1, &m_handle);
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
	ConstWeakArray<U32> families = getGrManagerImpl().getQueueFamilies();
	for(U32 i = 0; i < families.getSize(); ++i)
	{
		VkCommandPoolCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		ci.queueFamilyIndex = families[i];

		ANKI_VK_CHECK(vkCreateCommandPool(getVkDevice(), &ci, nullptr, &m_pools[i]));
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

	GpuQueueType queue;
	U32 queueFamilyIdx;
	if(!!(cmdbFlags & CommandBufferFlag::kGeneralWork) || getGrManagerImpl().getAsyncComputeType() == AsyncComputeType::kDisabled)
	{
		queue = GpuQueueType::kGeneral;
		queueFamilyIdx = 0;
	}
	else if(getGrManagerImpl().getAsyncComputeType() == AsyncComputeType::kLowPriorityQueue)
	{
		queue = GpuQueueType::kCompute;
		queueFamilyIdx = 0;
	}
	else
	{
		queue = GpuQueueType::kCompute;
		queueFamilyIdx = 1;
	}

	MicroObjectRecycler<MicroCommandBuffer>& recycler = m_recyclers[smallBatch][queueFamilyIdx];

	MicroCommandBuffer* out = recycler.findToReuse();

	if(out == nullptr) [[unlikely]]
	{
		// Create a new one

		VkCommandBufferAllocateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		ci.commandPool = m_pools[queueFamilyIdx];
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
	outPtr.reset(out);
	return Error::kNone;
}

void CommandBufferThreadAllocator::deleteCommandBuffer(MicroCommandBuffer* ptr)
{
	ANKI_ASSERT(ptr);

	const Bool smallBatch = !!(ptr->m_flags & CommandBufferFlag::kSmallBatch);

	const U32 queueFamilyIdx =
		(ptr->m_queue == GpuQueueType::kCompute && getGrManagerImpl().getAsyncComputeType() == AsyncComputeType::kLowPriorityQueue)
			? 0
			: U32(ptr->m_queue);

	m_recyclers[smallBatch][queueFamilyIdx].recycle(ptr);
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
			RLockGuard lock(m_threadAllocMtx);
			auto it = binarySearch(m_threadAllocs.getBegin(), m_threadAllocs.getEnd(), tid, Comp());
			alloc = (it != m_threadAllocs.getEnd()) ? (*it) : nullptr;
		}

		if(alloc == nullptr) [[unlikely]]
		{
			WLockGuard lock(m_threadAllocMtx);

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
