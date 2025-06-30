// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/D3D/D3DCommon.h>
#include <AnKi/Gr/Fence.h>
#include <AnKi/Gr/BackendCommon/MicroFenceFactory.h>

namespace anki {

/// @addtogroup directx
/// @{

/// Fence wrapper over D3D fence.
class MicroFenceImpl
{
public:
	~MicroFenceImpl()
	{
		ANKI_ASSERT(m_fence == nullptr);
	}

	explicit operator Bool() const
	{
		return m_fence != nullptr;
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

	void create()
	{
		ANKI_ASSERT(m_fence == nullptr && m_event == 0);
		ANKI_D3D_CHECKF(getDevice().CreateFence(m_value.getNonAtomically(), D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		m_event = CreateEvent(nullptr, false, false, nullptr);
		if(!m_event)
		{
			ANKI_D3D_LOGF("CreateEvent() failed");
		}
	}

	void destroy()
	{
		ANKI_ASSERT(m_fence && m_event);
		if(!CloseHandle(m_event))
		{
			ANKI_D3D_LOGE("CloseHandle() failed");
		}
		safeRelease(m_fence);
		m_fence = nullptr;
		m_event = 0;
	}

	void reset()
	{
		// Do nothing
	}

	void setName(CString)
	{
		// TODO
	}

	void gpuSignal(GpuQueueType queue);

	void gpuWait(GpuQueueType queue);

private:
	ID3D12Fence* m_fence = nullptr;
	Atomic<U64> m_value = 0;
	HANDLE m_event = 0;
};

using D3DMicroFence = MicroFence<MicroFenceImpl>;

/// Deleter for FencePtr.
class MicroFencePtrDeleter
{
public:
	void operator()(D3DMicroFence* f);
};

/// Fence smart pointer.
using MicroFencePtr = IntrusivePtr<D3DMicroFence, MicroFencePtrDeleter>;

/// A factory of fences.
class FenceFactory : public MakeSingleton<FenceFactory>
{
	friend class MicroFencePtrDeleter;

public:
	/// Create a new fence pointer.
	MicroFencePtr newInstance(CString name = "unnamed")
	{
		return MicroFencePtr(m_factory.newFence(name));
	}

private:
	MicroFenceFactory<MicroFenceImpl> m_factory;

	void deleteFence(D3DMicroFence* fence)
	{
		m_factory.releaseFence(fence);
	}
};

inline void MicroFencePtrDeleter::operator()(D3DMicroFence* fence)
{
	ANKI_ASSERT(fence);
	FenceFactory::getSingleton().deleteFence(fence);
}

/// Fence implementation.
class FenceImpl final : public Fence
{
public:
	MicroFencePtr m_fence;

	FenceImpl(CString name)
		: Fence(name)
	{
	}
};
/// @}

} // end namespace anki
