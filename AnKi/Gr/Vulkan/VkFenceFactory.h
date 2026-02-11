// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/VkCommon.h>
#include <AnKi/Gr/BackendCommon/MicroObjectRecycler.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Core/StatsSet.h>

namespace anki {

ANKI_SVAR(AliveFenceCount, StatCategory::kGr, "Current fence count", StatFlag::kNone)
ANKI_SVAR(FencesCreatedCount, StatCategory::kGr, "Total fences created", StatFlag::kNone)

// Fence wrapper over VkFence.
class MicroFence
{
public:
	MicroFence()
	{
		VkFenceCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		ANKI_VK_CHECKF(vkCreateFence(getVkDevice(), &ci, nullptr, &m_handle));
		g_svarAliveFenceCount.increment(1u);
		g_svarFencesCreatedCount.increment(1u);
	}

	~MicroFence()
	{
		ANKI_ASSERT(m_handle);
		vkDestroyFence(getVkDevice(), m_handle, nullptr);
		g_svarAliveFenceCount.decrement(1u);
	}

	void retain() const
	{
		m_refcount.fetchAdd(1);
	}

	void release();

	I32 getRefcount() const
	{
		return m_refcount.load();
	}

	Bool canRecycle() const
	{
		return signaled();
	}

	Bool clientWait(Second seconds) const
	{
		ANKI_ASSERT(m_handle);
		seconds = min<Second>(seconds, g_cvarGrGpuTimeout);
		const F64 nsf = 1e+9 * seconds;
		const U64 ns = U64(nsf);
		VkResult res;
		ANKI_VK_CHECKF(res = vkWaitForFences(getVkDevice(), 1, &m_handle, true, ns));

		return res != VK_TIMEOUT;
	}

	Bool signaled() const
	{
		ANKI_ASSERT(m_handle);
		VkResult status;
		ANKI_VK_CHECKF(status = vkGetFenceStatus(getVkDevice(), m_handle));
		return status == VK_SUCCESS;
	}

	void reset() const
	{
		ANKI_ASSERT(m_handle);
		ANKI_VK_CHECKF(vkResetFences(getVkDevice(), 1, &m_handle));
	}

	VkFence getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

private:
	VkFence m_handle = VK_NULL_HANDLE;
	mutable Atomic<I32> m_refcount = {0};
};

// Fence smart pointer.
using MicroFencePtr = IntrusiveNoDelPtr<MicroFence>;

// A factory of fences.
class FenceFactory : public MakeSingleton<FenceFactory>
{
	friend class MicroFence;

public:
	// Create a new fence pointer.
	MicroFencePtr newFence(CString name = "unnamed");

private:
	MicroObjectRecycler<MicroFence> m_recycler;

	void releaseFence(MicroFence* fence)
	{
		m_recycler.recycle(fence);
	}
};

inline void MicroFence::release()
{
	if(m_refcount.fetchSub(1) == 1)
	{
		FenceFactory::getSingleton().releaseFence(this);
	}
}

} // end namespace anki
