// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/SemaphoreFactory.h>

namespace anki
{

void SemaphoreFactory::destroy()
{
	for(U i = 0; i < m_semCount; ++i)
	{
		m_alloc.deleteInstance(m_sems[i]);
	}

	m_sems.destroy(m_alloc);
}

void SemaphoreFactory::releaseFences()
{
	U count = m_semCount;
	while(count--)
	{
		MicroSemaphore& sem = *m_sems[count];
		if(sem.m_fence && sem.m_fence->done())
		{
			sem.m_fence.reset(nullptr);
		}
	}
}

MicroSemaphorePtr SemaphoreFactory::newInstance(MicroFencePtr fence)
{
	ANKI_ASSERT(fence);

	LockGuard<Mutex> lock(m_mtx);

	MicroSemaphore* out = nullptr;

	if(m_semCount > 0)
	{
		releaseFences();

		U count = m_semCount;
		while(count--)
		{
			if(!m_sems[count]->m_fence)
			{
				out = m_sems[count];

				// Pop it
				for(U i = count; i < m_semCount - 1; ++i)
				{
					m_sems[i] = m_sems[i + 1];
				}

				--m_semCount;

				break;
			}
		}
	}

	if(out == nullptr)
	{
		// Create a new one
		out = m_alloc.newInstance<MicroSemaphore>(this, fence);
	}
	else
	{
		out->m_fence = fence;
	}

	ANKI_ASSERT(out->m_refcount.get() == 0);
	return MicroSemaphorePtr(out);
}

void SemaphoreFactory::destroySemaphore(MicroSemaphore* s)
{
	ANKI_ASSERT(s);
	ANKI_ASSERT(s->m_refcount.get() == 0);

	LockGuard<Mutex> lock(m_mtx);

	if(m_sems.getSize() <= m_semCount)
	{
		// Grow storage
		m_sems.resize(m_alloc, max<U>(1, m_sems.getSize() * 2));
	}

	m_sems[m_semCount] = s;
	++m_semCount;

	releaseFences();
}

} // end namespace anki
