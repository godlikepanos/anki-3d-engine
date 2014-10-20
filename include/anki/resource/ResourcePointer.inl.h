// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/ResourcePointer.h"

namespace anki {

//==============================================================================
template<typename T, typename TResourceManager>
Error ResourcePointer<T, TResourceManager>::load(
	const CString& filename, TResourceManager* resources)
{
	ANKI_ASSERT(m_cb == nullptr && "Already loaded");
	ANKI_ASSERT(resources != nullptr);

	Error err = ErrorCode::NONE;

	ResourcePointer other;
	Bool found = resources->_findLoadedResource(filename, other);

	if(!found)
	{
		auto alloc = resources->_getAllocator();

		// Allocate m_cb
		U len = filename.getLength();
		PtrSize alignment = alignof(ControlBlock);
		m_cb = reinterpret_cast<ControlBlock*>(
			alloc.allocate(
			sizeof(ControlBlock) + len, &alignment));

		if(!m_cb)
		{
			ANKI_LOGE("Out of memory when loading resource");
			return ErrorCode::OUT_OF_MEMORY;
		}

		// Construct
		alloc.construct(m_cb, alloc);

		// Populate the m_cb. Use a block ton cleanup temp_pool allocations
		auto& pool = resources->_getTempAllocator().getMemoryPool();

		// WARNING: Keep the brackets to force deallocation of newFname before
		// reseting the mempool
		{
			TempResourceString newFname(
				resources->fixResourceFilename(filename));

			ResourceInitializer init(
				alloc,
				resources->_getTempAllocator(),
				*resources);

			U allocsCountBefore = pool.getAllocationsCount();
			(void)allocsCountBefore;

			err = m_cb->m_resource.load(newFname.toCString(), init);
			if(err)
			{
				ANKI_LOGE("Failed to load resource: %s", &newFname[0]);
				alloc.deleteInstance(m_cb);
				m_cb = nullptr;
				return err;
			}

			ANKI_ASSERT(pool.getAllocationsCount() == allocsCountBefore
				&& "Forgot to deallocate");
		}

		m_cb->m_resources = resources;
		std::memcpy(&m_cb->m_uuid[0], &filename[0], len + 1);

		// Reset the memory pool if no-one is using it.
		// NOTE: Check because resources load other resources
		if(pool.getAllocationsCount() == 0)
		{
			pool.reset();
		}

		// Register resource
		resources->_registerResource(*this);
	}
	else
	{
		*this = other;
	}

	return err;
}

//==============================================================================
template<typename T, typename TResourceManager>
void ResourcePointer<T, TResourceManager>::reset()
{
	if(m_cb != nullptr)
	{
		auto count = m_cb->m_refcount.fetch_sub(1);
		if(count == 2)
		{
			m_cb->m_resources->_unregisterResource(*this);
		}
		else if(count == 1)
		{
			m_cb->m_resources->_getAllocator().deleteInstance(m_cb);
		}

		m_cb = nullptr;
	}
}

//==============================================================================
template<typename T, typename TResourceManager>
void ResourcePointer<T, TResourceManager>::copy(const ResourcePointer& b)
{
	reset();
	
	if(b.m_cb != nullptr)
	{
		auto count = b.m_cb->m_refcount.fetch_add(1);
		ANKI_ASSERT(count > 0);
		(void)count;

		m_cb = b.m_cb;
	}
}

} // end namespace anki

