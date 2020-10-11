// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/FenceFactory.h>
#include <anki/util/Tracer.h>

namespace anki
{

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
		vkDestroyFence(m_factory->m_dev, m_handle, nullptr);
	}
}

inline GrAllocator<U8> MicroFence::getAllocator() const
{
	return m_factory->m_alloc;
}

inline void MicroFence::wait()
{
	ANKI_ASSERT(m_handle);
	ANKI_VK_CHECKF(vkWaitForFences(m_factory->m_dev, 1, &m_handle, true, ~0U));
}

inline Bool MicroFence::done() const
{
	ANKI_ASSERT(m_handle);
	VkResult status = vkGetFenceStatus(m_factory->m_dev, m_handle);
	if(status == VK_SUCCESS)
	{
		return true;
	}
	else if(status != VK_NOT_READY)
	{
		ANKI_VK_LOGF("vkGetFenceStatus() failed");
	}

	return false;
}

inline Bool MicroFence::clientWait(Second seconds)
{
	if(seconds == 0.0)
	{
		return done();
	}
	else
	{
		VkResult res;
		F64 nsf = 1e+9 * seconds;
		U64 ns = U64(nsf);
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
