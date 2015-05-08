// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/ResourcePointer.h"

namespace anki {

//==============================================================================
template<typename T>
Error ResourcePointer<T>::load(
	const CString& filename, ResourceManager* resources)
{
	ANKI_ASSERT(!isCreated() && "Already loaded");
	ANKI_ASSERT(resources != nullptr);

	Error err = ErrorCode::NONE;

	T* other = resources->findLoadedResource<T>(filename);

	if(other)
	{
		// Found
		reset(other);
	}
	else
	{
		auto alloc = resources->getAllocator();

		// Allocate ptr
		T* ptr = alloc.newInstance<T>(resources);
		ANKI_ASSERT(ptr->getRefcount().load() == 0);

		// Populate the ptr. Use a block ton cleanup temp_pool allocations
		auto& pool = resources->getTempAllocator().getMemoryPool();

		// WARNING: Keep the brackets to force deallocation of newFname before
		// reseting the mempool
		{
			StringAuto newFname(resources->getTempAllocator());

			resources->fixResourceFilename(filename, newFname);

			U allocsCountBefore = pool.getAllocationsCount();
			(void)allocsCountBefore;

			err = ptr->load(newFname.toCString());
			if(err)
			{
				ANKI_LOGE("Failed to load resource: %s", &newFname[0]);
				alloc.deleteInstance(ptr);
				return err;
			}

			ANKI_ASSERT(pool.getAllocationsCount() == allocsCountBefore
				&& "Forgot to deallocate");
		}

		ptr->setUuid(filename);

		// Reset the memory pool if no-one is using it.
		// NOTE: Check because resources load other resources
		if(pool.getAllocationsCount() == 0)
		{
			pool.reset();
		}

		// Register resource
		resources->registerResource(ptr);
		reset(ptr);
	}

	return err;
}

//==============================================================================
template<typename T>
template<typename... TArgs>
Error ResourcePointer<T>::loadToCache(ResourceManager* resources, TArgs&&... args)
{
	StringAuto fname(resources->getTempAllocator());

	Error err = T::createToCache(args..., *resources, fname);

	if(!err)
	{
		err = load(fname.toCString(), resources);
	}

	return err;
}

} // end namespace anki

