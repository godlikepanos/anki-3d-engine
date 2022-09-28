// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/SemaphoreFactory.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

inline MicroSemaphore::MicroSemaphore(SemaphoreFactory* f, MicroFencePtr fence, Bool isTimeline)
	: m_factory(f)
	, m_fence(fence)
	, m_isTimeline(isTimeline)
{
	ANKI_ASSERT(f);
	ANKI_ASSERT(fence.isCreated());

	VkSemaphoreTypeCreateInfo typeCreateInfo = {};
	typeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
	typeCreateInfo.semaphoreType = (m_isTimeline) ? VK_SEMAPHORE_TYPE_TIMELINE : VK_SEMAPHORE_TYPE_BINARY;
	typeCreateInfo.initialValue = 0;

	VkSemaphoreCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	ci.pNext = &typeCreateInfo;

	ANKI_TRACE_INC_COUNTER(VK_SEMAPHORE_CREATE, 1);
	ANKI_VK_CHECKF(vkCreateSemaphore(m_factory->m_dev, &ci, nullptr, &m_handle));
}

inline MicroSemaphore::~MicroSemaphore()
{
	if(m_handle)
	{
		vkDestroySemaphore(m_factory->m_dev, m_handle, nullptr);
	}
}

inline HeapMemoryPool& MicroSemaphore::getMemoryPool()
{
	return *m_factory->m_pool;
}

inline Bool MicroSemaphore::clientWait(Second seconds)
{
	ANKI_ASSERT(m_isTimeline);

	seconds = min(seconds, kMaxFenceOrSemaphoreWaitTime);

	VkSemaphoreWaitInfo waitInfo = {};
	waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
	waitInfo.semaphoreCount = 1;
	waitInfo.pSemaphores = &m_handle;
	const U64 crntTimelineValue = m_timelineValue.load();
	waitInfo.pValues = &crntTimelineValue;

	const F64 nsf = 1e+9 * seconds;
	const U64 ns = U64(nsf);

	VkResult res;
	ANKI_VK_CHECKF(res = vkWaitSemaphoresKHR(m_factory->m_dev, &waitInfo, ns));

	return res != VK_TIMEOUT;
}

inline void MicroSemaphorePtrDeleter::operator()(MicroSemaphore* s)
{
	ANKI_ASSERT(s);
	if(s->m_isTimeline)
	{
		s->m_factory->m_timelineRecycler.recycle(s);
	}
	else
	{
		s->m_factory->m_binaryRecycler.recycle(s);
	}
}

inline MicroSemaphorePtr SemaphoreFactory::newInstance(MicroFencePtr fence, Bool isTimeline)
{
	ANKI_ASSERT(fence);

	MicroSemaphore* out = (isTimeline) ? m_timelineRecycler.findToReuse() : m_binaryRecycler.findToReuse();

	if(out == nullptr)
	{
		// Create a new one
		out = anki::newInstance<MicroSemaphore>(*m_pool, this, fence, isTimeline);
	}
	else
	{
		out->m_fence = fence;
		ANKI_ASSERT(out->m_isTimeline == isTimeline);
		if(out->m_isTimeline)
		{
			ANKI_ASSERT(out->m_timelineValue.getNonAtomically() > 0 && "Recycled without being signaled?");
		}
	}

	ANKI_ASSERT(out->m_refcount.getNonAtomically() == 0);
	return MicroSemaphorePtr(out);
}

} // end namespace anki
