// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/Common.h>

namespace anki {

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

	void retain() const
	{
		m_refcount.fetchAdd(1);
	}

	I32 release() const
	{
		return m_refcount.fetchSub(1);
	}

	void wait()
	{
		const Bool timeout = !clientWait(kMaxSecond);
		if(timeout) [[unlikely]]
		{
			ANKI_VK_LOGF("Waiting for a fence timed out");
		}
	}

	Bool clientWait(Second seconds);

	Bool done() const;

private:
	VkFence m_handle = VK_NULL_HANDLE;
	mutable Atomic<I32> m_refcount = {0};
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
	/// Limit the alive fences to avoid having too many file descriptors used in Linux.
	static constexpr U32 kMaxAliveFences = 32;

	FenceFactory() = default;

	~FenceFactory()
	{
		ANKI_ASSERT(m_fences.getSize() == 0);
	}

	void destroy();

	/// Create a new fence pointer.
	MicroFencePtr newInstance()
	{
		return MicroFencePtr(newFence());
	}

private:
	GrDynamicArray<MicroFence*> m_fences;
	U32 m_aliveFenceCount = 0;
	Mutex m_mtx;

	MicroFence* newFence();
	void deleteFence(MicroFence* fence);
};
/// @}

} // end namespace anki

#include <AnKi/Gr/Vulkan/FenceFactory.inl.h>
