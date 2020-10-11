// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/FenceFactory.h>
#include <anki/gr/CommandBuffer.h>
#include <anki/util/List.h>

namespace anki
{

// Forward
class CommandBufferFactory;
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
	}

	Atomic<I32>& getRefcount()
	{
		return m_refcount;
	}

	GrAllocator<U8>& getAllocator();

	StackAllocator<U8>& getFastAllocator()
	{
		return m_fastAlloc;
	}

	VkCommandBuffer getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

	template<typename T>
	void pushObjectRef(T& x)
	{
		GrObject* grobj = x.get();
		m_objectRefs.emplaceBack(m_fastAlloc, IntrusivePtr<GrObject>(grobj));
	}

	void setFence(MicroFencePtr& fence)
	{
		ANKI_ASSERT(!(m_flags & CommandBufferFlag::SECOND_LEVEL));
		ANKI_ASSERT(!m_fence.isCreated());
		m_fence = fence;
	}

private:
	StackAllocator<U8> m_fastAlloc;
	VkCommandBuffer m_handle = {};

	MicroFencePtr m_fence;
	DynamicArray<IntrusivePtr<GrObject>> m_objectRefs;

	// Cacheline boundary

	CommandBufferThreadAllocator* m_threadAlloc;
	Atomic<I32> m_refcount = {0};
	CommandBufferFlag m_flags = CommandBufferFlag::NONE;

	void destroy();
	void reset();
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
	CommandBufferThreadAllocator(CommandBufferFactory* factory, ThreadId tid)
		: m_factory(factory)
		, m_tid(tid)
	{
		ANKI_ASSERT(factory);
	}

	~CommandBufferThreadAllocator()
	{
	}

	ANKI_USE_RESULT Error init();

	void destroy();

	GrAllocator<U8>& getAllocator();

	/// Request a new command buffer.
	ANKI_USE_RESULT Error newCommandBuffer(CommandBufferFlag cmdbFlags, MicroCommandBufferPtr& ptr, Bool& createdNew);

	/// It will recycle it.
	void deleteCommandBuffer(MicroCommandBuffer* ptr);

private:
	CommandBufferFactory* m_factory;
	ThreadId m_tid;
	VkCommandPool m_pool = VK_NULL_HANDLE;

	class CmdbType
	{
	public:
		IntrusiveList<MicroCommandBuffer> m_readyCmdbs;
		IntrusiveList<MicroCommandBuffer> m_inUseCmdbs;

		IntrusiveList<MicroCommandBuffer> m_deletedCmdbs;
		Mutex m_deletedMtx; ///< Lock because the dallocations may happen anywhere.
	};

#if ANKI_EXTRA_CHECKS
	Atomic<U32> m_createdCmdbs = {0};
#endif

	Array2d<CmdbType, 2, 2> m_types;

	void destroyList(IntrusiveList<MicroCommandBuffer>& list);
	void destroyLists();
};

/// Command bufffer object recycler.
class CommandBufferFactory : public NonCopyable
{
	friend class CommandBufferThreadAllocator;
	friend class MicroCommandBuffer;

public:
	CommandBufferFactory() = default;

	~CommandBufferFactory() = default;

	ANKI_USE_RESULT Error init(GrAllocator<U8> alloc, VkDevice dev, uint32_t queueFamily);

	void destroy();

	/// Request a new command buffer.
	ANKI_USE_RESULT Error newCommandBuffer(ThreadId tid, CommandBufferFlag cmdbFlags, MicroCommandBufferPtr& ptr);

	/// Stats.
	U32 getCreatedCommandBufferCount() const
	{
		return m_createdCmdBufferCount.load();
	}

private:
	GrAllocator<U8> m_alloc;
	VkDevice m_dev = VK_NULL_HANDLE;
	uint32_t m_queueFamily;

	DynamicArray<CommandBufferThreadAllocator*> m_threadAllocs;
	RWMutex m_threadAllocMtx;

	Atomic<U32> m_createdCmdBufferCount = {0};
};
/// @}

} // end namespace anki

#include <anki/gr/vulkan/CommandBufferFactory.inl.h>
