// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/FenceFactory.h>
#include <anki/gr/vulkan/MicroObjectRecycler.h>

namespace anki
{

// Forward
class SemaphoreFactory;

/// @addtogroup vulkan
/// @{

/// Simple semaphore wrapper.
class MicroSemaphore : public NonCopyable
{
	friend class SemaphoreFactory;
	friend class MicroSemaphorePtrDeleter;
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

	MicroFencePtr& getFence()
	{
		return m_fence;
	}

private:
	VkSemaphore m_handle = VK_NULL_HANDLE;
	Atomic<U32> m_refcount = {0};
	SemaphoreFactory* m_factory = nullptr;

	/// Fence to find out when it's safe to reuse this semaphore.
	MicroFencePtr m_fence;

	MicroSemaphore(SemaphoreFactory* f, MicroFencePtr fence);

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
		m_recycler.init(alloc);
	}

	void destroy()
	{
		m_recycler.destroy();
	}

	MicroSemaphorePtr newInstance(MicroFencePtr fence);

private:
	GrAllocator<U8> m_alloc;
	VkDevice m_dev = VK_NULL_HANDLE;
	MicroObjectRecycler<MicroSemaphore> m_recycler;
};
/// @}

} // end namespace anki

#include <anki/gr/vulkan/SemaphoreFactory.inl.h>
