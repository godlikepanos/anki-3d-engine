// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/FenceFactory.h>
#include <AnKi/Gr/Vulkan/MicroObjectRecycler.h>

namespace anki
{

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

	Atomic<U32>& getRefcount()
	{
		return m_refcount;
	}

	GrAllocator<U8> getAllocator() const;

	void setFence(MicroFencePtr& f)
	{
		m_fence = f;
	}

	MicroFencePtr& getFence()
	{
		return m_fence;
	}

private:
	VkEvent m_handle = VK_NULL_HANDLE;
	Atomic<U32> m_refcount = {0};
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

	void init(GrAllocator<U8> alloc, VkDevice dev)
	{
		ANKI_ASSERT(dev);
		m_alloc = alloc;
		m_dev = dev;
	}

	void destroy()
	{
		m_recycler.destroy();
	}

	MicroDeferredBarrierPtr newInstance();

private:
	GrAllocator<U8> m_alloc;
	VkDevice m_dev = VK_NULL_HANDLE;
	MicroObjectRecycler<MicroDeferredBarrier> m_recycler;

	void destroyBarrier(MicroDeferredBarrier* barrier);
};
/// @}

} // end namespace anki

#include <AnKi/Gr/Vulkan/DeferredBarrierFactory.inl.h>
