// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/DeferredBarrierFactory.h>

namespace anki {

inline MicroDeferredBarrier::MicroDeferredBarrier(DeferredBarrierFactory* factory)
	: m_factory(factory)
{
	ANKI_ASSERT(factory);
	VkEventCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
	ANKI_VK_CHECKF(vkCreateEvent(factory->m_dev, &ci, nullptr, &m_handle));
}

inline MicroDeferredBarrier::~MicroDeferredBarrier()
{
	if(m_handle)
	{
		vkDestroyEvent(m_factory->m_dev, m_handle, nullptr);
		m_handle = VK_NULL_HANDLE;
	}
}

inline HeapMemoryPool& MicroDeferredBarrier::getMemoryPool()
{
	return *m_factory->m_pool;
}

inline void MicroDeferredBarrierPtrDeleter::operator()(MicroDeferredBarrier* s)
{
	ANKI_ASSERT(s);
	s->m_factory->m_recycler.recycle(s);
}

inline MicroDeferredBarrierPtr DeferredBarrierFactory::newInstance()
{
	MicroDeferredBarrier* out = m_recycler.findToReuse();

	if(out == nullptr)
	{
		// Create a new one
		out = anki::newInstance<MicroDeferredBarrier>(*m_pool, this);
	}

	return MicroDeferredBarrierPtr(out);
}

} // end namespace anki
