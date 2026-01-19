// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/VkCommon.h>
#include <AnKi/Gr/BackendCommon/MicroObjectRecycler.h>

namespace anki {

// Simple semaphore wrapper.
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

	Bool clientWait(Second seconds);

	Bool isTimeline() const
	{
		return m_isTimeline;
	}

	// Get the value of the semaphore after a signal.
	// Note: It's thread safe.
	U64 getNextSemaphoreValue()
	{
		ANKI_ASSERT(m_isTimeline);
		return m_timelineValue.fetchAdd(1) + 1;
	}

	// Get the value of the semaphore to wait on.
	// Note: It's thread safe.
	U64 getSemaphoreValue() const
	{
		ANKI_ASSERT(m_isTimeline);
		return m_timelineValue.load();
	}

private:
	VkSemaphore m_handle = VK_NULL_HANDLE;
	mutable Atomic<I32> m_refcount = {0};

	Atomic<U64> m_timelineValue = {0};
	Bool m_isTimeline = false;

	MicroSemaphore(Bool isTimeline);

	~MicroSemaphore();

	void releaseInternal();
};

// MicroSemaphore smart pointer.
using MicroSemaphorePtr = IntrusiveNoDelPtr<MicroSemaphore>;

// Factory of semaphores.
class SemaphoreFactory : public MakeSingleton<SemaphoreFactory>
{
	friend class MicroSemaphore;
	friend class MicroSemaphorePtrDeleter;

public:
	~SemaphoreFactory()
	{
		m_binaryRecycler.destroy();
		m_timelineRecycler.destroy();
	}

	MicroSemaphorePtr newInstance(Bool isTimeline, CString name);

private:
	MicroObjectRecycler<MicroSemaphore> m_binaryRecycler;
	MicroObjectRecycler<MicroSemaphore> m_timelineRecycler;
};

} // end namespace anki
