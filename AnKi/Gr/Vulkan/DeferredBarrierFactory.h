// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/FenceFactory.h>
#include <AnKi/Gr/Vulkan/MicroObjectRecycler.h>

namespace anki {

// Forward
class DeferredBarrierFactory;

/// @addtogroup vulkan
/// @{

/// Wrapper on top of VkEvent.
class MicroDeferredBarrier
{
	friend class DeferredBarrierFactory;
	friend class MicroDeferredBarrierPtrDeleter;

public:
	MicroDeferredBarrier(DeferredBarrierFactory* factory);

	~MicroDeferredBarrier();

	const VkEvent& getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

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

	HeapMemoryPool& getMemoryPool();

	void setFence(MicroFencePtr& f)
	{
		m_fence = f;
	}

	MicroFencePtr& getFence()
	{
		return m_fence;
	}

	/// Interface method.
	void onFenceDone()
	{
		// Do nothing
	}

private:
	VkEvent m_handle = VK_NULL_HANDLE;
	mutable Atomic<I32> m_refcount = {0};
	DeferredBarrierFactory* m_factory = nullptr;

	/// Fence to find out when it's safe to reuse this barrier.
	MicroFencePtr m_fence;
};

/// Deleter for MicroDeferredBarrierPtr smart pointer.
class MicroDeferredBarrierPtrDeleter
{
public:
	void operator()(MicroDeferredBarrier* x);
};

/// MicroDeferredBarrier smart pointer.
using MicroDeferredBarrierPtr = IntrusivePtr<MicroDeferredBarrier, MicroDeferredBarrierPtrDeleter>;

/// MicroDeferredBarrier factory.
class DeferredBarrierFactory
{
	friend class MicroDeferredBarrierPtrDeleter;
	friend class MicroDeferredBarrier;

public:
	DeferredBarrierFactory() = default;

	DeferredBarrierFactory(const DeferredBarrierFactory&) = delete; // Non-copyable

	DeferredBarrierFactory& operator=(const DeferredBarrierFactory&) = delete; // Non-copyable

	void init(HeapMemoryPool* pool, VkDevice dev)
	{
		ANKI_ASSERT(pool && dev);
		m_pool = pool;
		m_dev = dev;
	}

	void destroy()
	{
		m_recycler.destroy();
	}

	MicroDeferredBarrierPtr newInstance();

private:
	HeapMemoryPool* m_pool = nullptr;
	VkDevice m_dev = VK_NULL_HANDLE;
	MicroObjectRecycler<MicroDeferredBarrier> m_recycler;

	void destroyBarrier(MicroDeferredBarrier* barrier);
};
/// @}

} // end namespace anki

#include <AnKi/Gr/Vulkan/DeferredBarrierFactory.inl.h>
