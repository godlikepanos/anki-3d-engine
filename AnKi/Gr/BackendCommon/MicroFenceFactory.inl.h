// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/BackendCommon/MicroFenceFactory.h>
#include <AnKi/Core/StatsSet.h>

namespace anki {

ANKI_SVAR(FenceCount, StatCategory::kGr, "Fence count", StatFlag::kNone)

template<typename TImplementation>
MicroFenceFactory<TImplementation>::~MicroFenceFactory()
{
	GrDynamicArray<U32> fenceIdxToDelete;
	for(U32 idx = m_markedForDeletionMask.getLeastSignificantBit(); idx < kMaxU32; idx = m_markedForDeletionMask.getLeastSignificantBit())
	{
		fenceIdxToDelete.emplaceBack(idx);
		m_markedForDeletionMask.unset(idx);
	}

	for(U32 idx : fenceIdxToDelete)
	{
		MyMicroFence& fence = m_fences[idx];

		if(fence.m_impl)
		{
			fence.m_impl.destroy();
			ANKI_ASSERT(m_aliveFenceCount > 0);
			--m_aliveFenceCount;
			g_svarFenceCount.decrement(1_U64);
		}

		m_fences.erase(idx);
	}

	ANKI_ASSERT(!m_markedForDeletionMask.getAnySet());
	ANKI_ASSERT(m_fences.getSize() == 0);
	ANKI_ASSERT(m_aliveFenceCount == 0);
}

template<typename TImplementation>
MicroFenceFactory<TImplementation>::MyMicroFence* MicroFenceFactory<TImplementation>::newFence(CString name)
{
	MyMicroFence* out = nullptr;

	LockGuard<Mutex> lock(m_mtx);

	// Trim fences if needed
	if(m_aliveFenceCount > kMaxAliveFences * 80 / 100)
	{
		GrDynamicArray<U32> toDelete;
		for(auto it = m_fences.getBegin(); it != m_fences.getEnd(); ++it)
		{
			LockGuard lock(it->m_lock);

			const Bool markedForDeletion = m_markedForDeletionMask.get(it.getArrayIndex());

			if(markedForDeletion)
			{
				toDelete.emplaceBack(it.getArrayIndex());
			}

			if(!it->m_impl)
			{
				continue;
			}

			// Marked for deletion or signaled will loose their fence

			Bool destroyFence = it->m_state == MyMicroFence::kSignaled;
			if(!destroyFence)
			{
				const Bool signaled = it->m_impl.signaled();
				destroyFence = signaled;

				if(signaled)
				{
					it->m_state = MyMicroFence::kSignaled;
				}
			}

			if(destroyFence)
			{
				if(it->m_impl)
				{
					it->m_impl.destroy();
					ANKI_ASSERT(m_aliveFenceCount > 0);
					--m_aliveFenceCount;
					g_svarFenceCount.decrement(1_U64);
				}
			}
		}

		for(U32 i = 0; i < toDelete.getSize(); ++i)
		{
			m_fences.erase(toDelete[i]);
			ANKI_ASSERT(m_markedForDeletionMask.get(toDelete[i]));
			m_markedForDeletionMask.unset(toDelete[i]);
		}
	}

	// Find to recycle
	const U32 idx = m_markedForDeletionMask.getLeastSignificantBit();
	if(idx != kMaxU32)
	{
		m_markedForDeletionMask.unset(idx);
		out = &m_fences[idx];
	}

	if(!out)
	{
		// Create new
		auto it = m_fences.emplace();
		out = &(*it);
		out->m_arrayIdx = U16(it.getArrayIndex());
	}

	if(!out->m_impl)
	{
		// Create new

		out->m_impl.create();

		g_svarFenceCount.increment(1_U64);
		++m_aliveFenceCount;
		if(m_aliveFenceCount > kMaxAliveFences)
		{
			ANKI_GR_LOGW("Too many alive fences (%u). You may run out of file descriptors", m_aliveFenceCount);
		}
	}
	else
	{
		// Recycle
		if(!out->signaled())
		{
			ANKI_GR_LOGW("Fence marked for deletion but it's not signaled: %s", out->m_name.cstr());
		}

		out->m_impl.reset();
	}

	out->m_state = MyMicroFence::kUnsignaled;
	out->m_impl.setName(name);
	out->m_name = name;

	ANKI_ASSERT(out->m_refcount.getNonAtomically() == 0);
	return out;
}

template<typename TImplementation>
void MicroFenceFactory<TImplementation>::releaseFence(MyMicroFence* fence)
{
	LockGuard lock(m_mtx);

	ANKI_ASSERT(!m_markedForDeletionMask.get(fence->m_arrayIdx));
	m_markedForDeletionMask.set(fence->m_arrayIdx);

	if(!fence->signaled())
	{
		ANKI_GR_LOGW("Fence marked for deletion but it's not signaled: %s", fence->m_name.cstr());
	}
}

} // end namespace anki
