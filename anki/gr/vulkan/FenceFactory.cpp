// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/FenceFactory.h>

namespace anki
{

void FenceFactory::destroy()
{
	for(U32 i = 0; i < m_fenceCount; ++i)
	{
		m_alloc.deleteInstance(m_fences[i]);
	}

	m_fences.destroy(m_alloc);
}

MicroFence* FenceFactory::newFence()
{
	MicroFence* out = nullptr;

	LockGuard<Mutex> lock(m_mtx);

	U32 count = m_fenceCount;
	while(count--)
	{
		VkResult status;
		ANKI_VK_CHECKF(status = vkGetFenceStatus(m_dev, m_fences[count]->getHandle()));
		if(status == VK_SUCCESS)
		{
			out = m_fences[count];
			ANKI_VK_CHECKF(vkResetFences(m_dev, 1, &m_fences[count]->getHandle()));

			// Pop it
			for(U32 i = count; i < m_fenceCount - 1; ++i)
			{
				m_fences[i] = m_fences[i + 1];
			}

			--m_fenceCount;

			break;
		}
		else if(status != VK_NOT_READY)
		{
			ANKI_ASSERT(0);
		}
	}

	if(out == nullptr)
	{
		// Create a new one
		out = m_alloc.newInstance<MicroFence>(this);
	}

	ANKI_ASSERT(out->m_refcount.getNonAtomically() == 0);
	return out;
}

void FenceFactory::deleteFence(MicroFence* fence)
{
	ANKI_ASSERT(fence);

	LockGuard<Mutex> lock(m_mtx);

	if(m_fences.getSize() <= m_fenceCount)
	{
		// Grow storage
		m_fences.resize(m_alloc, max<U32>(1, m_fences.getSize() * 2));
	}

	m_fences[m_fenceCount] = fence;
	++m_fenceCount;
}

} // end namespace anki
