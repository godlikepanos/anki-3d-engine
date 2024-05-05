// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Fence.h>
#include <AnKi/Gr/D3D/D3DCommon.h>

namespace anki {

/// @addtogroup directx
/// @{

/// Fence wrapper over D3D fence.
class MicroFence
{
	friend class FenceFactory;

public:
	MicroFence();

	MicroFence(const MicroFence&) = delete; // Non-copyable

	~MicroFence()
	{
		ANKI_ASSERT(done());
		if(!CloseHandle(m_event))
		{
			ANKI_D3D_LOGE("CloseHandle() failed");
		}
		safeRelease(m_fence);
	}

	MicroFence& operator=(const MicroFence&) = delete; // Non-copyable

	void retain() const
	{
		m_refcount.fetchAdd(1);
	}

	I32 release() const
	{
		return m_refcount.fetchSub(1);
	}

	Bool done() const
	{
		const U64 cval = m_fence->GetCompletedValue();
		const U64 val = m_value.load();
		ANKI_ASSERT(cval <= val);
		return cval == val;
	}

	void gpuSignal(GpuQueueType queue);

	void gpuWait(GpuQueueType queue);

	Bool clientWait(Second seconds);

private:
	ID3D12Fence* m_fence = nullptr;
	Atomic<U64> m_value = 0;
	HANDLE m_event = 0;
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
	/// Limit the alive fences to avoid having too many file descriptors.
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

/// Fence implementation.
class FenceImpl final : public Fence
{
public:
	MicroFencePtr m_fence;

	FenceImpl(CString name)
		: Fence(name)
	{
	}

	~FenceImpl()
	{
	}
};
/// @}

} // end namespace anki
