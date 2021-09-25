// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/Common.h>

namespace anki
{

// Forward
class FenceFactory;

/// @addtogroup vulkan
/// @{

/// Fence wrapper over VkFence.
class MicroFence
{
	friend class FenceFactory;
	friend class MicroFencePtrDeleter;

public:
	MicroFence(FenceFactory* f);

	MicroFence(const MicroFence&) = delete; // Non-copyable

	~MicroFence();

	MicroFence& operator=(const MicroFence&) = delete; // Non-copyable

	const VkFence& getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

	Atomic<U32>& getRefcount()
	{
		return m_refcount;
	}

	GrAllocator<U8> getAllocator() const;

	void wait()
	{
		const Bool timeout = !clientWait(MAX_SECOND);
		if(ANKI_UNLIKELY(timeout))
		{
			ANKI_VK_LOGF("Waiting for a fence timed out");
		}
	}

	Bool clientWait(Second seconds);

	Bool done() const;

private:
	VkFence m_handle = VK_NULL_HANDLE;
	Atomic<U32> m_refcount = {0};
	FenceFactory* m_factory = nullptr;
};

/// Deleter for FencePtr.
class MicroFencePtrDeleter
{
public:
	void operator()(MicroFence* f);
};

/// Fence smart pointer.
using MicroFencePtr = IntrusivePtr<MicroFence, MicroFencePtrDeleter>;

/// A factory of fences.
class FenceFactory
{
	friend class MicroFence;
	friend class MicroFencePtrDeleter;

public:
	FenceFactory()
	{
	}

	~FenceFactory()
	{
	}

	void init(GrAllocator<U8> alloc, VkDevice dev)
	{
		ANKI_ASSERT(dev);
		m_alloc = alloc;
		m_dev = dev;
	}

	void destroy();

	/// Create a new fence pointer.
	MicroFencePtr newInstance()
	{
		return MicroFencePtr(newFence());
	}

private:
	GrAllocator<U8> m_alloc;
	VkDevice m_dev = VK_NULL_HANDLE;
	DynamicArray<MicroFence*> m_fences;
	U32 m_fenceCount = 0;
	Mutex m_mtx;

	MicroFence* newFence();
	void deleteFence(MicroFence* fence);
};
/// @}

} // end namespace anki

#include <AnKi/Gr/Vulkan/FenceFactory.inl.h>
