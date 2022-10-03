// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/Common.h>
#include <AnKi/Util/DynamicArray.h>

namespace anki {

/// @addtogroup vulkan
/// @{

/// Helper class for MicroXXX objects. It expects a specific interface for the T.
template<typename T>
class MicroObjectRecycler
{
public:
	MicroObjectRecycler()
	{
	}

	MicroObjectRecycler(HeapMemoryPool* pool)
	{
		init(pool);
	}

	~MicroObjectRecycler()
	{
		destroy();
	}

	void init(HeapMemoryPool* pool)
	{
		m_pool = pool;
	}

	/// It's thread-safe.
	void destroy();

	/// Find a new one to reuse. It's thread-safe.
	T* findToReuse();

	/// Release an object back to the recycler. It's thread-safe.
	void recycle(T* s);

	/// Destroy those objects that their fence is done. It's thread-safe.
	void trimCache()
	{
		LockGuard<Mutex> lock(m_mtx);
		checkDoneFences();
		trimCacheInternal(0);
	}

	U32 getCacheSize() const
	{
		return m_objects.getSize();
	}

private:
	class Object
	{
	public:
		T* m_microObject;
		Bool m_fenceDone;
	};

	HeapMemoryPool* m_pool = nullptr;
	DynamicArray<Object> m_objects;
	Mutex m_mtx;

	// Begin trim cache adjustment vars
	U32 m_readyObjectsAfterTrim = 1;
	static constexpr U32 m_maxRequestsPerAdjustment = 128;
	U32 m_cacheMisses = 0;
	U32 m_requests = 0;
	U32 m_minCacheSizePerRequest = kMaxU32;
	// End trim cache adjustment vars

#if ANKI_EXTRA_CHECKS
	U32 m_createdAndNotRecycled = 0;
#endif

	void trimCacheInternal(U32 aliveObjectCountAfterTrim);

	void adjustAliveObjectCount();

	void checkDoneFences();
};
/// @}

} // end namespace anki

#include <AnKi/Gr/Vulkan/MicroObjectRecycler.inl.h>
