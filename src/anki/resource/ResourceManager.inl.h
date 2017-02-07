// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/ResourceManager.h>

namespace anki
{

template<typename T>
Error ResourceManager::loadResource(const CString& filename, ResourcePtr<T>& out)
{
	ANKI_ASSERT(!out.isCreated() && "Already loaded");

	Error err = ErrorCode::NONE;
	++m_loadRequestCount;

	T* other = findLoadedResource<T>(filename);

	if(other)
	{
		// Found
		out.reset(other);
	}
	else
	{
		// Allocate ptr
		T* ptr = m_alloc.newInstance<T>(this);
		ANKI_ASSERT(ptr->getRefcount().load() == 0);

		// Populate the ptr. Use a block to cleanup temp_pool allocations
		auto& pool = m_tmpAlloc.getMemoryPool();

		{
			U allocsCountBefore = pool.getAllocationsCount();
			(void)allocsCountBefore;

			err = ptr->load(filename);
			if(err)
			{
				ANKI_RESOURCE_LOGE("Failed to load resource: %s", &filename[0]);
				m_alloc.deleteInstance(ptr);
				return err;
			}

			ANKI_ASSERT(pool.getAllocationsCount() == allocsCountBefore && "Forgot to deallocate");
		}

		ptr->setFilename(filename);
		ptr->setUuid(++m_uuid);

		// Reset the memory pool if no-one is using it.
		// NOTE: Check because resources load other resources
		if(pool.getAllocationsCount() == 0)
		{
			pool.reset();
		}

		// Register resource
		registerResource(ptr);
		out.reset(ptr);
	}

	return err;
}

template<typename T, typename... TArgs>
Error ResourceManager::loadResourceToCache(ResourcePtr<T>& out, TArgs&&... args)
{
	StringAuto fname(m_tmpAlloc);

	Error err = T::createToCache(args..., *this, fname);

	if(!err)
	{
		err = loadResource(fname.toCString(), out);
	}

	return err;
}

} // end namespace anki
