// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/BackendCommon/Common.h>
#include <AnKi/Util/DynamicArray.h>

namespace anki {

// Helper class for MicroXXX objects. It expects a specific interface for the T:
// I32 getRefcount() const;
// Bool canRecycle() const;
template<typename T>
class MicroObjectRecycler
{
public:
	MicroObjectRecycler()
	{
	}

	~MicroObjectRecycler()
	{
		destroy();
	}

	// It's thread-safe.
	void destroy();

	// Find a new one to reuse. It's thread-safe.
	T* findToReuse();

	// Release an object back to the recycler. It's thread-safe.
	void recycle(T* s);

	// Destroy those objects that their fence is done. It's thread-safe.
	void trimCache()
	{
		LockGuard<Mutex> lock(m_mtx);
		trimCacheInternal(0);
	}

	U32 getCacheSize() const
	{
		return m_objectCache.getSize();
	}

private:
	GrDynamicArray<T*> m_objectCache;
	Mutex m_mtx;

	// Begin trim cache adjustment vars
	U32 m_availableObjectsAfterTrim = 1;
	static constexpr U32 kMaxRequestsPerAdjustment = 128;
	U32 m_cacheMisses = 0;
	U32 m_requests = 0;
	// End trim cache adjustment vars

#if ANKI_ASSERTIONS_ENABLED
	U32 m_inUseObjects = 0;
#endif

	void trimCacheInternal(U32 aliveObjectCountAfterTrim);

	void adjustAliveObjectCount();
};

} // end namespace anki

#include <AnKi/Gr/BackendCommon/MicroObjectRecycler.inl.h>
