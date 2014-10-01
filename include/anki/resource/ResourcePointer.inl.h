// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/ResourcePointer.h"

namespace anki {

//==============================================================================
template<typename T, typename TResourceManager>
void ResourcePointer<T, TResourceManager>::load(
	const CString& filename, TResourceManager* resources)
{
	ANKI_ASSERT(m_cb == nullptr && "Already loaded");
	ANKI_ASSERT(resources != nullptr);

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

		// Construct
		try
		{
			alloc.construct(m_cb, alloc);
		}
		catch(const std::exception& e)
		{
			alloc.deallocate(m_cb, 1);
			m_cb = nullptr;
			throw ANKI_EXCEPTION("Control block construction failed") << e;
		}

		// Populate the m_cb. Use a block ton cleanup temp_pool allocations
		auto& pool = resources->_getTempAllocator().getMemoryPool();

		{
			TempResourceString newFname(
				resources->fixResourceFilename(filename));

			try
			{
				ResourceInitializer init(
					alloc,
					resources->_getTempAllocator(),
					*resources);

				U allocsCountBefore = pool.getAllocationsCount();
				(void)allocsCountBefore;

				m_cb->m_resource.load(newFname.toCString(), init);

				ANKI_ASSERT(pool.getAllocationsCount() == allocsCountBefore
					&& "Forgot to deallocate");
			}
			catch(const std::exception& e)
			{
				alloc.deleteInstance(m_cb);
				m_cb = nullptr;
				throw ANKI_EXCEPTION("Loading failed: %s", &newFname[0]) << e;
			}
		}

		m_cb->m_resources = resources;
		std::memcpy(&m_cb->m_uuid[0], &filename[0], len + 1);

		// Reset the memory pool if no-one is using it
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

