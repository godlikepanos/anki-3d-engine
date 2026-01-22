// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/BackendCommon/MicroObjectRecycler.h>

namespace anki {

template<typename T>
inline void MicroObjectRecycler<T>::destroy()
{
	LockGuard<Mutex> lock(m_mtx);

	for(U32 i = 0; i < m_objectCache.getSize(); ++i)
	{
		T* mobj = m_objectCache[i];
		ANKI_ASSERT(mobj);
		deleteInstance(GrMemoryPool::getSingleton(), mobj);
	}

	m_objectCache.destroy();
	ANKI_ASSERT(m_inUseObjects == 0 && "Destroying the recycler while objects have not recycled yet");
}

template<typename T>
inline T* MicroObjectRecycler<T>::findToReuse()
{
	T* out = nullptr;
	LockGuard<Mutex> lock(m_mtx);

	adjustAliveObjectCount();

	// Trim the cache but leave at least one object to be recycled
	trimCacheInternal(max(m_availableObjectsAfterTrim, 1u));

	for(auto it = m_objectCache.getBegin(); it != m_objectCache.getEnd(); ++it)
	{
		if((*it)->canRecycle())
		{
			out = *it;
			m_objectCache.erase(it);
			break;
		}
	}

	ANKI_ASSERT(out == nullptr || out->getRefcount() == 0);

	m_cacheMisses += (out == nullptr);

#if ANKI_ASSERTIONS_ENABLED
	++m_inUseObjects;
#endif

	return out;
}

template<typename T>
void MicroObjectRecycler<T>::recycle(T* mobj)
{
	ANKI_ASSERT(mobj);
	ANKI_ASSERT(mobj->getRefcount() == 0);

	LockGuard<Mutex> lock(m_mtx);

	m_objectCache.emplaceBack(mobj);
	trimCacheInternal(m_availableObjectsAfterTrim);

#if ANKI_ASSERTIONS_ENABLED
	ANKI_ASSERT(m_inUseObjects > 0);
	--m_inUseObjects;
#endif
}

template<typename T>
void MicroObjectRecycler<T>::trimCacheInternal(U32 aliveObjectCountAfterTrim)
{
	aliveObjectCountAfterTrim = min(aliveObjectCountAfterTrim, m_objectCache.getSize());
	U32 toBeKilledCount = m_objectCache.getSize() - aliveObjectCountAfterTrim;
	if(toBeKilledCount == 0)
	{
		return;
	}

	GrDynamicArray<T*> newObjectCache;
	for(U32 i = 0; i < m_objectCache.getSize(); ++i)
	{
		if(toBeKilledCount > 0 && m_objectCache[i]->canRecycle())
		{
			deleteInstance(GrMemoryPool::getSingleton(), m_objectCache[i]);
			--toBeKilledCount;
		}
		else
		{
			newObjectCache.emplaceBack(m_objectCache[i]);
		}
	}

	m_objectCache.destroy();
	m_objectCache = std::move(newObjectCache);
}

template<typename T>
void MicroObjectRecycler<T>::adjustAliveObjectCount()
{
	if(m_requests < kMaxRequestsPerAdjustment) [[likely]]
	{
		// Not enough requests, keep getting stats
		++m_requests;
	}
	else
	{
		constexpr U32 kGrowCount = 4;
		constexpr U32 kMinAvailableObjects = 1;

		if(m_cacheMisses)
		{
			// Need more alive objects
			m_availableObjectsAfterTrim += kGrowCount;
		}
		else if(m_availableObjectsAfterTrim > kMinAvailableObjects)
		{
			// Have more than enough alive objects per request, decrease alive objects
			--m_availableObjectsAfterTrim;
		}

		// Start new cycle
		m_cacheMisses = 0;
		m_requests = 0;
	}
}

} // end namespace anki
