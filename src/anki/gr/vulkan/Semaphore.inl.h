// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/Semaphore.h>
#include <anki/core/Trace.h>

namespace anki
{

inline Semaphore::Semaphore(SemaphoreFactory* f, FencePtr fence)
	: m_factory(f)
	, m_fence(fence)
{
	ANKI_ASSERT(f);
	ANKI_ASSERT(fence.isCreated());
	VkSemaphoreCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	ANKI_TRACE_INC_COUNTER(VK_SEMAPHORE_CREATE, 1);
	ANKI_VK_CHECKF(vkCreateSemaphore(m_factory->m_dev, &ci, nullptr, &m_handle));
}

inline Semaphore::~Semaphore()
{
	if(m_handle)
	{
		vkDestroySemaphore(m_factory->m_dev, m_handle, nullptr);
	}
}

inline void SemaphorePtrDeleter::operator()(Semaphore* s)
{
	ANKI_ASSERT(s);
	s->m_factory->destroySemaphore(s);
}

} // end namespace anki
