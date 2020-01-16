// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/SemaphoreFactory.h>
#include <anki/util/Tracer.h>

namespace anki
{

inline MicroSemaphore::MicroSemaphore(SemaphoreFactory* f, MicroFencePtr fence)
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

inline MicroSemaphore::~MicroSemaphore()
{
	if(m_handle)
	{
		vkDestroySemaphore(m_factory->m_dev, m_handle, nullptr);
	}
}

inline GrAllocator<U8> MicroSemaphore::getAllocator() const
{
	return m_factory->m_alloc;
}

inline void MicroSemaphorePtrDeleter::operator()(MicroSemaphore* s)
{
	ANKI_ASSERT(s);
	s->m_factory->m_recycler.recycle(s);
}

inline MicroSemaphorePtr SemaphoreFactory::newInstance(MicroFencePtr fence)
{
	ANKI_ASSERT(fence);

	MicroSemaphore* out = m_recycler.findToReuse();

	if(out == nullptr)
	{
		// Create a new one
		out = m_alloc.newInstance<MicroSemaphore>(this, fence);
	}
	else
	{
		out->m_fence = fence;
	}

	ANKI_ASSERT(out->m_refcount.getNonAtomically() == 0);
	return MicroSemaphorePtr(out);
}

} // end namespace anki
