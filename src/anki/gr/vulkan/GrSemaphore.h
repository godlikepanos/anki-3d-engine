// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/Fence.h>

namespace anki
{

// Forward
class GrSemaphoreFactory;

/// @addtogroup vulkan
/// @{

/// Simple semaphore wrapper.
class GrSemaphore : public NonCopyable
{
	friend class GrSemaphoreFactory;
	friend class GrSemaphorePtrDeleter;
	template<typename, typename>
	friend class GenericPoolAllocator;

public:
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

private:
	VkSemaphore m_handle = VK_NULL_HANDLE;
	Atomic<U32> m_refcount = {0};
	GrSemaphoreFactory* m_factory = nullptr;

	/// Fence to find out when it's safe to reuse this semaphore.
	FencePtr m_fence;

	GrSemaphore(GrSemaphoreFactory* f, FencePtr fence);

	~GrSemaphore();
};

/// GrSemaphorePtr deleter.
class GrSemaphorePtrDeleter
{
public:
	void operator()(GrSemaphore* s);
};

/// GrSemaphore smart pointer.
using GrSemaphorePtr = IntrusivePtr<GrSemaphore, GrSemaphorePtrDeleter>;

/// Factory of semaphores.
class GrSemaphoreFactory
{
	friend class GrSemaphore;
	friend class GrSemaphorePtrDeleter;

public:
	void init(GrAllocator<U8> alloc, VkDevice dev)
	{
		ANKI_ASSERT(dev);
		m_alloc = alloc;
		m_dev = dev;
	}

	void destroy();

	GrSemaphorePtr newInstance(FencePtr fence);

private:
	GrAllocator<U8> m_alloc;
	DynamicArray<GrSemaphore*> m_sems;
	U32 m_semCount = 0;
	VkDevice m_dev = VK_NULL_HANDLE;
	Mutex m_mtx;

	void destroySemaphore(GrSemaphore* s);

	/// Iterate the semaphores and release the fences.
	void releaseFences();
};
/// @}

} // end namespace anki

#include <anki/gr/vulkan/GrSemaphore.inl.h>
