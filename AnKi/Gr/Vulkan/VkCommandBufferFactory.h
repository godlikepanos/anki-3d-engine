// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/VkFenceFactory.h>
#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Gr/BackendCommon/MicroObjectRecycler.h>
#include <AnKi/Gr/Vulkan/VkDescriptor.h>
#include <AnKi/Util/List.h>

namespace anki {

// Forward
class CommandBufferThreadAllocator;

/// @addtogroup vulkan
/// @{

class MicroCommandBuffer
{
	friend class CommandBufferThreadAllocator;

public:
	MicroCommandBuffer(CommandBufferThreadAllocator* allocator)
		: m_threadAlloc(allocator)
	{
		ANKI_ASSERT(allocator);
	}

	~MicroCommandBuffer();

	void retain() const
	{
		m_refcount.fetchAdd(1);
	}

	void release()
	{
		if(m_refcount.fetchSub(1) == 1)
		{
			releaseInternal();
		}
	}

	I32 getRefcount() const
	{
		return m_refcount.load();
	}

	Bool canRecycle() const
	{
		return true;
	}

	VkCommandBuffer getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

	CommandBufferFlag getFlags() const
	{
		return m_flags;
	}

	GpuQueueType getVulkanQueueType() const
	{
		ANKI_ASSERT(m_queue != GpuQueueType::kCount);
		return m_queue;
	}

	DescriptorAllocator& getDSAllocator()
	{
		return m_dsAllocator;
	}

private:
	VkCommandBuffer m_handle = {};

	DescriptorAllocator m_dsAllocator;

	CommandBufferThreadAllocator* m_threadAlloc;
	mutable Atomic<I32> m_refcount = {0};
	CommandBufferFlag m_flags = CommandBufferFlag::kNone;
	GpuQueueType m_queue = GpuQueueType::kCount;

	void releaseInternal();
};

/// Micro command buffer pointer.
using MicroCommandBufferPtr = IntrusiveNoDelPtr<MicroCommandBuffer>;

/// Per-thread command buffer allocator.
class alignas(ANKI_CACHE_LINE_SIZE) CommandBufferThreadAllocator
{
	friend class CommandBufferFactory;
	friend class MicroCommandBuffer;

public:
	CommandBufferThreadAllocator(ThreadId tid)
		: m_tid(tid)
	{
	}

	~CommandBufferThreadAllocator()
	{
	}

	Error init();

	void destroy();

	/// Request a new command buffer.
	Error newCommandBuffer(CommandBufferFlag cmdbFlags, MicroCommandBufferPtr& ptr);

	/// It will recycle it.
	void recycleCommandBuffer(MicroCommandBuffer* ptr);

private:
	ThreadId m_tid;
	Array<VkCommandPool, U(GpuQueueType::kCount)> m_pools = {};

#if ANKI_EXTRA_CHECKS
	Atomic<U32> m_createdCmdbs = {0};
#endif

	Array2d<MicroObjectRecycler<MicroCommandBuffer>, 2, U(GpuQueueType::kCount)> m_recyclers;
};

/// Command bufffer object recycler.
class CommandBufferFactory : public MakeSingleton<CommandBufferFactory>
{
	friend class CommandBufferThreadAllocator;
	friend class MicroCommandBuffer;

public:
	CommandBufferFactory() = default;

	CommandBufferFactory(const CommandBufferFactory&) = delete; // Non-copyable

	~CommandBufferFactory()
	{
		destroy();
	}

	CommandBufferFactory& operator=(const CommandBufferFactory&) = delete; // Non-copyable

	/// Request a new command buffer.
	Error newCommandBuffer(ThreadId tid, CommandBufferFlag cmdbFlags, MicroCommandBufferPtr& ptr);

private:
	GrDynamicArray<CommandBufferThreadAllocator*> m_threadAllocs;
	RWMutex m_threadAllocMtx;

	void destroy();
};
/// @}

} // end namespace anki
