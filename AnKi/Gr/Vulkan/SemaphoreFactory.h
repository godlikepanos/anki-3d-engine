// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/FenceFactory.h>
#include <AnKi/Gr/Vulkan/MicroObjectRecycler.h>

namespace anki {

// Forward
class SemaphoreFactory;

/// @addtogroup vulkan
/// @{

/// Simple semaphore wrapper.
class MicroSemaphore
{
	friend class SemaphoreFactory;
	friend class MicroSemaphorePtrDeleter;
	ANKI_FRIEND_CALL_CONSTRUCTOR_AND_DESTRUCTOR

public:
	MicroSemaphore(const MicroSemaphore&) = delete; // Non-copyable

	MicroSemaphore& operator=(const MicroSemaphore&) = delete; // Non-copyable

	const VkSemaphore& getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

	HeapMemoryPool& getMemoryPool();

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

	MicroFencePtr& getFence()
	{
		return m_fence;
	}

	/// Interface method.
	void onFenceDone()
	{
		// Do nothing
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
	mutable Atomic<I32> m_refcount = {0};
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
	void init(HeapMemoryPool* pool, VkDevice dev)
	{
		ANKI_ASSERT(dev);
		m_pool = pool;
		m_dev = dev;
		m_binaryRecycler.init(m_pool);
		m_timelineRecycler.init(m_pool);
	}

	void destroy()
	{
		m_binaryRecycler.destroy();
		m_timelineRecycler.destroy();
	}

	MicroSemaphorePtr newInstance(MicroFencePtr fence, Bool isTimeline);

private:
	HeapMemoryPool* m_pool = nullptr;
	VkDevice m_dev = VK_NULL_HANDLE;
	MicroObjectRecycler<MicroSemaphore> m_binaryRecycler;
	MicroObjectRecycler<MicroSemaphore> m_timelineRecycler;
};
/// @}

} // end namespace anki

#include <AnKi/Gr/Vulkan/SemaphoreFactory.inl.h>
