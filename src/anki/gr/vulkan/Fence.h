// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/Common.h>

namespace anki
{

// Forward
class FenceFactory;

/// @addtogroup vulkan
/// @{

/// Fence wrapper over VkFence.
class Fence : public NonCopyable
{
	friend class FenceFactory;
	friend class FencePtrDeleter;

public:
	Fence(FenceFactory* f);

	~Fence();

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

	void wait();

	Bool done() const;

private:
	VkFence m_handle = VK_NULL_HANDLE;
	Atomic<U32> m_refcount = {0};
	FenceFactory* m_factory = nullptr;
};

/// Deleter for FencePtr.
class FencePtrDeleter
{
public:
	void operator()(Fence* f);
};

/// Fence smart pointer.
using FencePtr = IntrusivePtr<Fence, FencePtrDeleter>;

/// A factory of fences.
class FenceFactory
{
	friend class Fence;
	friend class FencePtrDeleter;

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
	FencePtr newInstance()
	{
		return FencePtr(newFence());
	}

private:
	GrAllocator<U8> m_alloc;
	VkDevice m_dev = VK_NULL_HANDLE;
	DynamicArray<Fence*> m_fences;
	U32 m_fenceCount = 0;
	Mutex m_mtx;

	Fence* newFence();
	void deleteFence(Fence* fence);
};
/// @}

} // end namespace anki

#include <anki/gr/vulkan/Fence.inl.h>
