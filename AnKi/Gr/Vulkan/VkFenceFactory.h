// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/VkCommon.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

/// @addtogroup vulkan
/// @{

/// Fence wrapper over VkFence.
class MicroFence
{
	friend class FenceFactory;
	friend class MicroFencePtrDeleter;

public:
	MicroFence()
	{
		VkFenceCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		ANKI_TRACE_INC_COUNTER(VkFenceCreate, 1);
		ANKI_VK_CHECKF(vkCreateFence(getVkDevice(), &ci, nullptr, &m_handle));
	}

	MicroFence(const MicroFence&) = delete; // Non-copyable

	~MicroFence()
	{
		if(m_handle)
		{
			ANKI_ASSERT(done());
			vkDestroyFence(getVkDevice(), m_handle, nullptr);
		}
	}

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

	Bool clientWait(Second seconds)
	{
		ANKI_ASSERT(m_handle);

		if(seconds == 0.0)
		{
			return done();
		}
		else
		{
			seconds = min(seconds, kMaxFenceOrSemaphoreWaitTime);
			const F64 nsf = 1e+9 * seconds;
			const U64 ns = U64(nsf);
			VkResult res;
			ANKI_VK_CHECKF(res = vkWaitForFences(getVkDevice(), 1, &m_handle, true, ns));

			return res != VK_TIMEOUT;
		}
	}

	Bool done() const
	{
		ANKI_ASSERT(m_handle);

		VkResult status;
		ANKI_VK_CHECKF(status = vkGetFenceStatus(getVkDevice(), m_handle));
		return status == VK_SUCCESS;
	}

private:
	VkFence m_handle = VK_NULL_HANDLE;
	mutable Atomic<I32> m_refcount = {0};
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
class FenceFactory : public MakeSingleton<FenceFactory>
{
	friend class MicroFence;
	friend class MicroFencePtrDeleter;

public:
	/// Limit the alive fences to avoid having too many file descriptors used in Linux.
	static constexpr U32 kMaxAliveFences = 32;

	FenceFactory() = default;

	~FenceFactory();

	/// Create a new fence pointer.
	MicroFencePtr newInstance()
	{
		return MicroFencePtr(newFence());
	}

	void trimSignaledFences(Bool wait);

private:
	GrDynamicArray<MicroFence*> m_fences;
	U32 m_aliveFenceCount = 0;
	Mutex m_mtx;

	MicroFence* newFence();
	void deleteFence(MicroFence* fence);
};
/// @}

} // end namespace anki
