// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/VkFenceFactory.h>

namespace anki {

void MicroFencePtrDeleter::operator()(MicroFence* f)
{
	ANKI_ASSERT(f);
	FenceFactory::getSingleton().deleteFence(f);
}

FenceFactory::~FenceFactory()
{
	trimSignaledFences(true);

	ANKI_ASSERT(m_fences.getSize() == 0);
	ANKI_ASSERT(m_aliveFenceCount == 0);
}

MicroFence* FenceFactory::newFence()
{
	MicroFence* out = nullptr;

	{
		LockGuard<Mutex> lock(m_mtx);

		for(U32 i = 0; i < m_fences.getSize(); ++i)
		{
			VkResult status;
			ANKI_VK_CHECKF(status = vkGetFenceStatus(getVkDevice(), m_fences[i]->getHandle()));
			if(status == VK_SUCCESS)
			{
				out = m_fences[i];

				// Pop it
				m_fences.erase(m_fences.getBegin() + i);
				break;
			}
			else if(status != VK_NOT_READY)
			{
				ANKI_ASSERT(0);
			}
		}

		if(out)
		{
			// Recycle
		}
		else
		{
			// Create new

			++m_aliveFenceCount;

			if(m_aliveFenceCount > kMaxAliveFences)
			{
				ANKI_VK_LOGW("Too many alive fences (%u). You may run out of file descriptors", m_aliveFenceCount);
			}
		}
	}

	if(out == nullptr)
	{
		// Create a new one
		out = anki::newInstance<MicroFence>(GrMemoryPool::getSingleton());
	}
	else
	{
		// Recycle
		ANKI_VK_CHECKF(vkResetFences(getVkDevice(), 1, &out->getHandle()));
	}

	ANKI_ASSERT(out->m_refcount.getNonAtomically() == 0);
	return out;
}

void FenceFactory::deleteFence(MicroFence* fence)
{
	ANKI_ASSERT(fence);

	LockGuard<Mutex> lock(m_mtx);
	m_fences.emplaceBack(fence);
}

void FenceFactory::trimSignaledFences(Bool wait)
{
	LockGuard<Mutex> lock(m_mtx);

	GrDynamicArray<MicroFence*> unsignaledFences;
	for(MicroFence* fence : m_fences)
	{
		const Bool signaled = fence->clientWait((wait) ? g_gpuTimeoutCVar : 0.0);
		if(signaled)
		{
			deleteInstance(GrMemoryPool::getSingleton(), fence);

			ANKI_ASSERT(m_aliveFenceCount > 0);
			--m_aliveFenceCount;
		}
		else
		{
			unsignaledFences.emplaceBack(fence);
		}
	}

	m_fences.destroy();
	if(unsignaledFences.getSize())
	{
		m_fences = std::move(unsignaledFences);
	}
}

} // end namespace anki
