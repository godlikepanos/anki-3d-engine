// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/VkFenceFactory.h>
#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Gr/BackendCommon/MicroObjectRecycler.h>
#include <AnKi/Gr/Vulkan/VkDescriptorSetFactory.h>
#include <AnKi/Util/List.h>

namespace anki {

// Forward
class CommandBufferThreadAllocator;

/// @addtogroup vulkan
/// @{

class MicroCommandBuffer : public IntrusiveListEnabled<MicroCommandBuffer>
{
	friend class CommandBufferThreadAllocator;
	friend class MicroCommandBufferPtrDeleter;

public:
	MicroCommandBuffer(CommandBufferThreadAllocator* allocator)
		: m_threadAlloc(allocator)
	{
		ANKI_ASSERT(allocator);
		for(DynamicArray<GrObjectPtr, MemoryPoolPtrWrapper<StackMemoryPool>>& arr : m_objectRefs)
		{
			arr = DynamicArray<GrObjectPtr, MemoryPoolPtrWrapper<StackMemoryPool>>(&m_fastPool);
		}
	}

	~MicroCommandBuffer();

	void retain() const
	{
		m_refcount.fetchAdd(1);
	}

	I32 release() const
	{
		return m_refcount.fetchSub(1);
	}

	I32 getRefcount() const
	{
		return m_refcount.load();
	}

	void setFence(MicroFence* fence)
	{
		m_fence.reset(fence);
	}

	MicroFence* getFence() const
	{
		return m_fence.tryGet();
	}

	/// Interface method.
	void onFenceDone()
	{
		reset();
	}

	StackMemoryPool& getFastMemoryPool()
	{
		return m_fastPool;
	}

	VkCommandBuffer getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

	template<typename T>
	void pushObjectRef(T* x)
	{
		ANKI_ASSERT(T::kClassType != GrObjectType::kTexture && T::kClassType != GrObjectType::kTextureView && T::kClassType != GrObjectType::kBuffer
					&& "No need to push references of buffers and textures");
		pushToArray(m_objectRefs[T::kClassType], x);
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

	DSAllocator& getDSAllocator()
	{
		return m_dsAllocator;
	}

private:
	static constexpr U32 kMaxRefObjectSearch = 16;

	StackMemoryPool m_fastPool;
	VkCommandBuffer m_handle = {};

	MicroFencePtr m_fence;
	Array<DynamicArray<GrObjectPtr, MemoryPoolPtrWrapper<StackMemoryPool>>, U(GrObjectType::kCount)> m_objectRefs;

	DSAllocator m_dsAllocator;

	CommandBufferThreadAllocator* m_threadAlloc;
	mutable Atomic<I32> m_refcount = {0};
	CommandBufferFlag m_flags = CommandBufferFlag::kNone;
	GpuQueueType m_queue = GpuQueueType::kCount;

	void reset();

	void pushToArray(DynamicArray<GrObjectPtr, MemoryPoolPtrWrapper<StackMemoryPool>>& arr, GrObject* grobj)
	{
		ANKI_ASSERT(grobj);

		// Search the temp cache to avoid setting the ref again
		if(arr.getSize() >= kMaxRefObjectSearch)
		{
			for(U32 i = arr.getSize() - kMaxRefObjectSearch; i < arr.getSize(); ++i)
			{
				if(arr[i].get() == grobj)
				{
					return;
				}
			}
		}

		// Not found in the temp cache, add it
		arr.emplaceBack(grobj);
	}
};

/// Deleter.
class MicroCommandBufferPtrDeleter
{
public:
	void operator()(MicroCommandBuffer* buff);
};

/// Micro command buffer pointer.
using MicroCommandBufferPtr = IntrusivePtr<MicroCommandBuffer, MicroCommandBufferPtrDeleter>;

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
	void deleteCommandBuffer(MicroCommandBuffer* ptr);

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
	CommandBufferFactory(const VulkanQueueFamilies& queueFamilies)
		: m_queueFamilies(queueFamilies)
	{
	}

	CommandBufferFactory(const CommandBufferFactory&) = delete; // Non-copyable

	~CommandBufferFactory()
	{
		destroy();
	}

	CommandBufferFactory& operator=(const CommandBufferFactory&) = delete; // Non-copyable

	/// Request a new command buffer.
	Error newCommandBuffer(ThreadId tid, CommandBufferFlag cmdbFlags, MicroCommandBufferPtr& ptr);

private:
	VulkanQueueFamilies m_queueFamilies;

	GrDynamicArray<CommandBufferThreadAllocator*> m_threadAllocs;
	RWMutex m_threadAllocMtx;

	void destroy();
};
/// @}

} // end namespace anki
