// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/MicroObjectRecycler.h>

namespace anki {

template<typename T>
inline void MicroObjectRecycler<T>::destroy()
{
	LockGuard<Mutex> lock(m_mtx);

	checkDoneFences();

	for(U32 i = 0; i < m_objects.getSize(); ++i)
	{
		T* mobj = m_objects[i].m_microObject;
		ANKI_ASSERT(mobj);
		ANKI_ASSERT(!mobj->getFence());

		auto& pool = mobj->getMemoryPool();
		deleteInstance(pool, mobj);
#if ANKI_EXTRA_CHECKS
		--m_createdAndNotRecycled;
#endif
	}

	m_objects.destroy(*m_pool);
	ANKI_ASSERT(m_createdAndNotRecycled == 0 && "Destroying the recycler while objects have not recycled yet");
}

template<typename T>
inline T* MicroObjectRecycler<T>::findToReuse()
{
	T* out = nullptr;
	LockGuard<Mutex> lock(m_mtx);

	checkDoneFences();
	adjustAliveObjectCount();

	// Trim the cache but leave at least one object to be recycled
	trimCacheInternal(max(m_readyObjectsAfterTrim, 1u));

	for(U32 i = 0; i < m_objects.getSize(); ++i)
	{
		if(m_objects[i].m_fenceDone)
		{
			out = m_objects[i].m_microObject;
			m_objects[i] = m_objects[m_objects.getSize() - 1];
			m_objects.popBack(*m_pool);

			break;
		}
	}

	ANKI_ASSERT(out == nullptr || out->getRefcount() == 0);

	m_cacheMisses += (out == nullptr);

#if ANKI_EXTRA_CHECKS
	if(out == nullptr)
	{
		++m_createdAndNotRecycled;
	}
#endif

	return out;
}

template<typename T>
void MicroObjectRecycler<T>::recycle(T* mobj)
{
	ANKI_ASSERT(mobj);
	ANKI_ASSERT(mobj->getRefcount() == 0);

	LockGuard<Mutex> lock(m_mtx);

	Object obj;
	obj.m_fenceDone = !mobj->getFence();
	obj.m_microObject = mobj;

	if(obj.m_fenceDone)
	{
		mobj->onFenceDone();
	}

	m_objects.emplaceBack(*m_pool, obj);
	checkDoneFences();
	trimCacheInternal(m_readyObjectsAfterTrim);
}

template<typename T>
void MicroObjectRecycler<T>::checkDoneFences()
{
	for(Object& obj : m_objects)
	{
		T& mobj = *obj.m_microObject;

		if(obj.m_fenceDone)
		{
			ANKI_ASSERT(!mobj.getFence());
		}

		if(!obj.m_fenceDone && mobj.getFence() && mobj.getFence()->done())
		{
			mobj.getFence().reset(nullptr);
			mobj.onFenceDone();
			obj.m_fenceDone = true;
		}
	}
}

template<typename T>
void MicroObjectRecycler<T>::trimCacheInternal(U32 aliveObjectCountAfterTrim)
{
	DynamicArray<Object> aliveObjects;

	for(Object& obj : m_objects)
	{
		T& mobj = *obj.m_microObject;
		const Bool inUseByTheGpu = !obj.m_fenceDone;

		if(inUseByTheGpu)
		{
			// Can't delete it for sure
			aliveObjects.emplaceBack(*m_pool, obj);
		}
		else if(aliveObjectCountAfterTrim > 0)
		{
			// Need to keep a few alive for recycling
			aliveObjects.emplaceBack(*m_pool, obj);
			--aliveObjectCountAfterTrim;
		}
		else
		{
			auto& pool = mobj.getMemoryPool();
			deleteInstance(pool, &mobj);
#if ANKI_EXTRA_CHECKS
			--m_createdAndNotRecycled;
#endif
		}
	}

	if(aliveObjects.getSize() > 0)
	{
		// Some alive, store the alive
		m_objects.destroy(*m_pool);
		m_objects = std::move(aliveObjects);
	}
	else if(aliveObjects.getSize() == 0 && m_objects.getSize() > 0)
	{
		// All dead, destroy the array
		m_objects.destroy(*m_pool);
	}
}

template<typename T>
void MicroObjectRecycler<T>::adjustAliveObjectCount()
{
	U32 readyObjects = 0;
	for(Object& obj : m_objects)
	{
		readyObjects += obj.m_fenceDone;
	}

	if(ANKI_LIKELY(m_requests < m_maxRequestsPerAdjustment))
	{
		// Not enough requests for a recycle
		m_minCacheSizePerRequest = min(m_minCacheSizePerRequest, readyObjects);
		++m_requests;
	}
	else
	{
		if(m_cacheMisses)
		{
			// Need more alive objects
			m_readyObjectsAfterTrim += 4;
		}
		else if(m_minCacheSizePerRequest > 2 && m_readyObjectsAfterTrim > 0)
		{
			// Have more than enough alive objects per request, decrease alive objects
			--m_readyObjectsAfterTrim;
		}

		// Start new cycle
		m_cacheMisses = 0;
		m_requests = 0;
		m_minCacheSizePerRequest = readyObjects;
	}
}

} // end namespace anki
