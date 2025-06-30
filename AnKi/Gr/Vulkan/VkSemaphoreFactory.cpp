// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/VkSemaphoreFactory.h>
#include <AnKi/Gr/Vulkan/VkGrManager.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

MicroSemaphore::MicroSemaphore(MicroFencePtr fence, Bool isTimeline)
	: m_fence(fence)
	, m_isTimeline(isTimeline)
{
	ANKI_ASSERT(fence.isCreated());

	VkSemaphoreTypeCreateInfo typeCreateInfo = {};
	typeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
	typeCreateInfo.semaphoreType = (m_isTimeline) ? VK_SEMAPHORE_TYPE_TIMELINE : VK_SEMAPHORE_TYPE_BINARY;
	typeCreateInfo.initialValue = 0;

	VkSemaphoreCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	ci.pNext = &typeCreateInfo;

	ANKI_TRACE_INC_COUNTER(VkSemaphoreCreate, 1);
	ANKI_VK_CHECKF(vkCreateSemaphore(getVkDevice(), &ci, nullptr, &m_handle));
}

MicroSemaphore::~MicroSemaphore()
{
	if(m_handle)
	{
		vkDestroySemaphore(getVkDevice(), m_handle, nullptr);
	}
}

Bool MicroSemaphore::clientWait(Second seconds)
{
	ANKI_ASSERT(m_isTimeline);

	seconds = min<Second>(seconds, g_gpuTimeoutCVar);

	VkSemaphoreWaitInfo waitInfo = {};
	waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
	waitInfo.semaphoreCount = 1;
	waitInfo.pSemaphores = &m_handle;
	const U64 crntTimelineValue = m_timelineValue.load();
	waitInfo.pValues = &crntTimelineValue;

	const F64 nsf = 1e+9 * seconds;
	const U64 ns = U64(nsf);

	VkResult res;
	ANKI_VK_CHECKF(res = vkWaitSemaphores(getVkDevice(), &waitInfo, ns));

	return res != VK_TIMEOUT;
}

void MicroSemaphorePtrDeleter::operator()(MicroSemaphore* s)
{
	ANKI_ASSERT(s);
	if(s->m_isTimeline)
	{
		SemaphoreFactory::getSingleton().m_timelineRecycler.recycle(s);
	}
	else
	{
		SemaphoreFactory::getSingleton().m_binaryRecycler.recycle(s);
	}
}

MicroSemaphorePtr SemaphoreFactory::newInstance(MicroFencePtr fence, Bool isTimeline, CString name)
{
	ANKI_ASSERT(fence);

	MicroSemaphore* out = (isTimeline) ? m_timelineRecycler.findToReuse() : m_binaryRecycler.findToReuse();

	if(out == nullptr)
	{
		// Create a new one
		out = anki::newInstance<MicroSemaphore>(GrMemoryPool::getSingleton(), fence, isTimeline);
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

	getGrManagerImpl().trySetVulkanHandleName(name, VK_OBJECT_TYPE_SEMAPHORE, out->m_handle);

	ANKI_ASSERT(out->m_refcount.getNonAtomically() == 0);
	return MicroSemaphorePtr(out);
}

} // end namespace anki
