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

/// Helper class for MicroXXX objects.
template<typename T>
class MicroObjectRecycler
{
public:
	MicroObjectRecycler()
	{
	}

	MicroObjectRecycler(GrAllocator<U8> alloc)
	{
		init(alloc);
	}

	~MicroObjectRecycler()
	{
		destroy();
	}

	void init(GrAllocator<U8> alloc)
	{
		m_alloc = alloc;
	}

	/// It's thread-safe.
	void destroy();

	/// Find a new one to reuse. It's thread-safe.
	T* findToReuse();

	/// Release an object back to the recycler. It's thread-safe.
	void recycle(T* s);

	/// Destroy those objects that their fence is done. It's thread-safe.
	/// @param objectsToNotDestroy The number of objects to keep alive for future recycling.
	void trimCache(U32 objectsToNotDestroy = 0);

private:
	GrAllocator<U8> m_alloc;
	DynamicArray<T*> m_objects;
	Mutex m_mtx;
#if ANKI_EXTRA_CHECKS
	U32 m_createdAndNotRecycled = 0;
#endif

	/// @return The number of objects that could be deleted.
	U32 releaseFences();
};
/// @}

} // end namespace anki

#include <AnKi/Gr/Vulkan/MicroObjectRecycler.inl.h>
