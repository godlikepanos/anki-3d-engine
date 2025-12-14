// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/BackendCommon/Common.h>
#include <AnKi/Util/BlockArray.h>

namespace anki {

/// @addtogroup graphics
/// @{

template<typename TImplementation>
class MicroFence
{
public:
	template<typename>
	friend class MicroFenceFactory;

public:
	MicroFence() = default;

	MicroFence(const MicroFence&) = delete; // Non-copyable

	MicroFence& operator=(const MicroFence&) = delete; // Non-copyable

	const TImplementation& getImplementation() const
	{
		LockGuard lock(m_lock);
		ANKI_ASSERT(m_state == kUnsignaled);
		return m_impl;
	}

	TImplementation& getImplementation()
	{
		LockGuard lock(m_lock);
		ANKI_ASSERT(m_state == kUnsignaled);
		return m_impl;
	}

	void retain() const
	{
		m_refcount.fetchAdd(1);
	}

	I32 release() const
	{
		return m_refcount.fetchSub(1);
	}

	Bool clientWait(Second seconds)
	{
		LockGuard lock(m_lock);

		Bool bSignaled;
		if(m_state == kSignaled)
		{
			bSignaled = true;
		}
		else if(seconds == 0.0)
		{
			bSignaled = m_impl.signaled();
		}
		else
		{
			// Not signaled and has handle
			seconds = min<Second>(seconds, g_cvarGrGpuTimeout);
			bSignaled = m_impl.clientWait(seconds);
		}

		if(bSignaled)
		{
			m_state = kSignaled;
		}

		return bSignaled;
	}

	Bool signaled()
	{
		LockGuard lock(m_lock);

		Bool bSignaled;
		if(m_state == kSignaled)
		{
			bSignaled = true;
		}
		else
		{
			bSignaled = m_impl.signaled();
			if(bSignaled)
			{
				m_state = kSignaled;
			}
		}

		return bSignaled;
	}

	CString getName() const
	{
		return m_name;
	}

private:
	enum State : U8
	{
		kUnsignaled,
		kSignaled
	};

	TImplementation m_impl;
	mutable Atomic<I32> m_refcount = {0};
	mutable SpinLock m_lock;
	U16 m_arrayIdx = kMaxU16;
	State m_state = kUnsignaled;
	GrString m_name;
};

/// A factory of fences. It's designed to minimize the number of alive GFX API fences. If creating many VkFences on Linux the application may run out
/// of file descriptors.
template<typename TImplementation>
class MicroFenceFactory
{
public:
	using MyMicroFence = MicroFence<TImplementation>;

	/// Limit the alive fences to avoid having too many file descriptors used in Linux.
	static constexpr U32 kMaxAliveFences = 32;

	MicroFenceFactory() = default;

	~MicroFenceFactory();

	/// Create a new fence pointer.
	MyMicroFence* newFence(CString name = "unnamed");

	void releaseFence(MyMicroFence* fence);

private:
	GrBlockArray<MyMicroFence> m_fences;
	U32 m_aliveFenceCount = 0;
	Mutex m_mtx;
	BitSet<1024, U64> m_markedForDeletionMask = {false}; // Always last for better caching
};
/// @}

} // end namespace anki

#include <AnKi/Gr/BackendCommon/MicroFenceFactory.inl.h>
