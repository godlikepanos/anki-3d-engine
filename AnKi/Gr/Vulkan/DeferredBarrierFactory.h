// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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
	MicroDeferredBarrier(DeferredBarrierFactory* factory)
		: m_factory(factory)
	{
		ANKI_ASSERT(factory);
		VkEventCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
		ANKI_VK_CHECKF(vkCreateEvent(getVkDevice(), &ci, nullptr, &m_handle));
	}

	~MicroDeferredBarrier()
	{
		if(m_handle)
		{
			vkDestroyEvent(getVkDevice(), m_handle, nullptr);
			m_handle = VK_NULL_HANDLE;
		}
	}

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

	void destroy()
	{
		m_recycler.destroy();
	}

	MicroDeferredBarrierPtr newInstance()
	{
		MicroDeferredBarrier* out = m_recycler.findToReuse();

		if(out == nullptr)
		{
			// Create a new one
			out = anki::newInstance<MicroDeferredBarrier>(GrMemoryPool::getSingleton(), this);
		}

		return MicroDeferredBarrierPtr(out);
	}

private:
	MicroObjectRecycler<MicroDeferredBarrier> m_recycler;

	void destroyBarrier(MicroDeferredBarrier* barrier);
};

inline void MicroDeferredBarrierPtrDeleter::operator()(MicroDeferredBarrier* s)
{
	ANKI_ASSERT(s);
	s->m_factory->m_recycler.recycle(s);
}
/// @}

} // end namespace anki
