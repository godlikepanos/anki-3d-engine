// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/DeferredBarrierFactory.h>

namespace anki
{

void DeferredBarrierFactory::destroy()
{
	U count = m_barrierCount;
	while(count--)
	{
		m_alloc.deleteInstance(m_barriers[count]);
	}

	m_barriers.destroy(m_alloc);
}

void DeferredBarrierFactory::releaseFences()
{
	U count = m_barrierCount;
	while(count--)
	{
		MicroDeferredBarrier& b = *m_barriers[count];
		if(b.m_fence && b.m_fence->done())
		{
			b.m_fence.reset(nullptr);
		}
	}
}

MicroDeferredBarrierPtr DeferredBarrierFactory::newInstance()
{
	LockGuard<Mutex> lock(m_mtx);

	MicroDeferredBarrier* out = nullptr;

	if(m_barrierCount > 0)
	{
		releaseFences();

		U count = m_barrierCount;
		while(count--)
		{
			if(!m_barriers[count]->m_fence)
			{
				out = m_barriers[count];

				// Pop it
				for(U i = count; i < m_barrierCount - 1; ++i)
				{
					m_barriers[i] = m_barriers[i + 1];
				}

				--m_barrierCount;

				break;
			}
		}
	}

	if(out == nullptr)
	{
		// Create a new one
		out = m_alloc.newInstance<MicroDeferredBarrier>(this);
	}

	ANKI_ASSERT(out->m_refcount.get() == 0);
	return MicroDeferredBarrierPtr(out);
}

void DeferredBarrierFactory::destroyBarrier(MicroDeferredBarrier* s)
{
	ANKI_ASSERT(s);
	ANKI_ASSERT(s->m_refcount.get() == 0);

	LockGuard<Mutex> lock(m_mtx);

	if(m_barriers.getSize() <= m_barrierCount)
	{
		// Grow storage
		m_barriers.resize(m_alloc, max<U>(1, m_barriers.getSize() * 2));
	}

	m_barriers[m_barrierCount] = s;
	++m_barrierCount;

	releaseFences();
}

} // end namespace anki