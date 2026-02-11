// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/D3D/D3DCommon.h>
#include <AnKi/Gr/Fence.h>
#include <AnKi/Gr/BackendCommon/MicroObjectRecycler.h>
#include <AnKi/Core/StatsSet.h>

namespace anki {

ANKI_SVAR(AliveFenceCount, StatCategory::kGr, "Current fence count", StatFlag::kNone)
ANKI_SVAR(FencesCreatedCount, StatCategory::kGr, "Total fences created", StatFlag::kNone)

// Fence wrapper over D3D fence.
class MicroFence
{
public:
	MicroFence()
	{
		ANKI_D3D_CHECKF(getDevice().CreateFence(m_value.getNonAtomically(), D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		m_event = CreateEvent(nullptr, false, false, nullptr);
		if(!m_event)
		{
			ANKI_D3D_LOGF("CreateEvent() failed");
		}
		g_svarAliveFenceCount.increment(1u);
		g_svarFencesCreatedCount.increment(1u);
	}

	~MicroFence()
	{
		if(!CloseHandle(m_event))
		{
			ANKI_D3D_LOGE("CloseHandle() failed");
		}
		safeRelease(m_fence);
		m_fence = nullptr;
		m_event = 0;

		g_svarAliveFenceCount.decrement(1u);
	}

	Bool clientWait(Second seconds)
	{
		ANKI_ASSERT(m_fence && m_event);
		ANKI_D3D_CHECKF(m_fence->SetEventOnCompletion(m_value.load(), m_event));

		const DWORD msec = DWORD(seconds * 1000.0);
		const DWORD result = WaitForSingleObjectEx(m_event, msec, FALSE);

		Bool signaled = false;
		if(result == WAIT_OBJECT_0)
		{
			signaled = true;
		}
		else if(result == WAIT_TIMEOUT)
		{
			signaled = false;
		}
		else
		{
			ANKI_D3D_LOGF("WaitForSingleObjectEx() returned %u", result);
		}

		return signaled;
	}

	Bool signaled() const
	{
		ANKI_ASSERT(m_fence);
		const U64 cval = m_fence->GetCompletedValue();
		ANKI_D3D_CHECKF((cval < kMaxU64) ? S_OK : DXGI_ERROR_DEVICE_REMOVED);
		const U64 val = m_value.load();
		ANKI_ASSERT(cval <= val);
		return cval == val;
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

	void setName(CString)
	{
		// TODO
	}

	void reset()
	{
		// To nothing
	}

	void gpuSignal(GpuQueueType queue);

	void gpuWait(GpuQueueType queue);

private:
	ID3D12Fence* m_fence = nullptr;
	Atomic<U64> m_value = 0;
	HANDLE m_event = 0;
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

// Fence implementation.
class FenceImpl final : public Fence
{
public:
	MicroFencePtr m_fence;

	FenceImpl(CString name)
		: Fence(name)
	{
	}
};

} // end namespace anki
