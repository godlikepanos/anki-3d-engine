// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/FenceFactory.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

inline MicroFence::MicroFence(FenceFactory* f)
	: m_factory(f)
{
	ANKI_ASSERT(f);
	VkFenceCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	ANKI_TRACE_INC_COUNTER(VK_FENCE_CREATE, 1);
	ANKI_VK_CHECKF(vkCreateFence(m_factory->m_dev, &ci, nullptr, &m_handle));
}

inline MicroFence::~MicroFence()
{
	if(m_handle)
	{
		ANKI_ASSERT(done());
		vkDestroyFence(m_factory->m_dev, m_handle, nullptr);
	}
}

inline HeapMemoryPool& MicroFence::getMemoryPool()
{
	return *m_factory->m_pool;
}

inline Bool MicroFence::done() const
{
	ANKI_ASSERT(m_handle);
	VkResult status;
	ANKI_VK_CHECKF(status = vkGetFenceStatus(m_factory->m_dev, m_handle));
	return status == VK_SUCCESS;
}

inline Bool MicroFence::clientWait(Second seconds)
{
	ANKI_ASSERT(m_handle);

	if(seconds == 0.0)
	{
		return done();
	}
	else
	{
		seconds = min(seconds, kMaxFenceOrSemaphoreWaitTime);

		const F64 nsf = 1e+9 * seconds;
		const U64 ns = U64(nsf);
		VkResult res;
		ANKI_VK_CHECKF(res = vkWaitForFences(m_factory->m_dev, 1, &m_handle, true, ns));

		return res != VK_TIMEOUT;
	}
}

inline void MicroFencePtrDeleter::operator()(MicroFence* f)
{
	ANKI_ASSERT(f);
	f->m_factory->deleteFence(f);
}

} // end namespace anki
