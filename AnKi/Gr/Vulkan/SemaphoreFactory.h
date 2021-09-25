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
class SemaphoreFactory;

/// @addtogroup vulkan
/// @{

/// Simple semaphore wrapper.
class MicroSemaphore
{
	friend class SemaphoreFactory;
	friend class MicroSemaphorePtrDeleter;
	template<typename, typename>
	friend class GenericPoolAllocator;

public:
	MicroSemaphore(const MicroSemaphore&) = delete; // Non-copyable

	MicroSemaphore& operator=(const MicroSemaphore&) = delete; // Non-copyable

	const VkSemaphore& getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

	GrAllocator<U8> getAllocator() const;

	Atomic<U32>& getRefcount()
	{
		return m_refcount;
	}

	MicroFencePtr& getFence()
	{
		return m_fence;
	}

	void setFence(MicroFencePtr& fence)
	{
		m_fence = fence;
	}

	Bool clientWait(Second seconds);

	Bool isTimeline() const
	{
		return m_isTimeline;
	}

	/// Get the value of the semaphore after a signal.
	/// @note It's thread safe.
	U64 getNextSemaphoreValue()
	{
		ANKI_ASSERT(m_isTimeline);
		return m_timelineValue.fetchAdd(1) + 1;
	}

	/// Get the value of the semaphore to wait on.
	/// @note It's thread safe.
	U64 getSemaphoreValue() const
	{
		ANKI_ASSERT(m_isTimeline);
		return m_timelineValue.load();
	}

private:
	VkSemaphore m_handle = VK_NULL_HANDLE;
	Atomic<U32> m_refcount = {0};
	SemaphoreFactory* m_factory = nullptr;

	/// Fence to find out when it's safe to reuse this semaphore.
	MicroFencePtr m_fence;

	Atomic<U64> m_timelineValue = {0};
	Bool m_isTimeline = false;

	MicroSemaphore(SemaphoreFactory* f, MicroFencePtr fence, Bool isTimeline);

	~MicroSemaphore();
};

/// MicroSemaphorePtr deleter.
class MicroSemaphorePtrDeleter
{
public:
	void operator()(MicroSemaphore* s);
};

/// MicroSemaphore smart pointer.
using MicroSemaphorePtr = IntrusivePtr<MicroSemaphore, MicroSemaphorePtrDeleter>;

/// Factory of semaphores.
class SemaphoreFactory
{
	friend class MicroSemaphore;
	friend class MicroSemaphorePtrDeleter;

public:
	void init(GrAllocator<U8> alloc, VkDevice dev)
	{
		ANKI_ASSERT(dev);
		m_alloc = alloc;
		m_dev = dev;
		m_binaryRecycler.init(alloc);
		m_timelineRecycler.init(alloc);
	}

	void destroy()
	{
		m_binaryRecycler.destroy();
		m_timelineRecycler.destroy();
	}

	MicroSemaphorePtr newInstance(MicroFencePtr fence, Bool isTimeline);

private:
	GrAllocator<U8> m_alloc;
	VkDevice m_dev = VK_NULL_HANDLE;
	MicroObjectRecycler<MicroSemaphore> m_binaryRecycler;
	MicroObjectRecycler<MicroSemaphore> m_timelineRecycler;
};
/// @}

} // end namespace anki

#include <AnKi/Gr/Vulkan/SemaphoreFactory.inl.h>
