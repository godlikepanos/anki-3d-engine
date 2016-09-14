// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/Fence.h>

namespace anki
{

// Forward
class SemaphoreFactory;

/// @addtogroup vulkan
/// @{

/// Simple semaphore wrapper.
class Semaphore : public NonCopyable
{
	friend class SemaphoreFactory;
	friend class SemaphorePtrDeleter;
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
	SemaphoreFactory* m_factory = nullptr;

	/// Fence to find out when it's safe to reuse this semaphore.
	FencePtr m_fence;

	Semaphore(SemaphoreFactory* f, FencePtr fence);

	~Semaphore();
};

/// SemaphorePtr deleter.
class SemaphorePtrDeleter
{
public:
	void operator()(Semaphore* s);
};

/// Semaphore smart pointer.
using SemaphorePtr = IntrusivePtr<Semaphore, SemaphorePtrDeleter>;

/// Factory of semaphores.
class SemaphoreFactory
{
	friend class Semaphore;
	friend class SemaphorePtrDeleter;

public:
	void init(GrAllocator<U8> alloc, VkDevice dev)
	{
		ANKI_ASSERT(dev);
		m_alloc = alloc;
		m_dev = dev;
	}

	void destroy();

	SemaphorePtr newInstance(FencePtr fence);

private:
	GrAllocator<U8> m_alloc;
	DynamicArray<Semaphore*> m_sems;
	U32 m_semCount = 0;
	VkDevice m_dev = VK_NULL_HANDLE;
	Mutex m_mtx;

	void destroySemaphore(Semaphore* s);

	/// Iterate the semaphores and release the fences.
	void releaseFences();
};
/// @}

} // end namespace anki

#include <anki/gr/vulkan/Semaphore.inl.h>
