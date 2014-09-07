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
	ANKI_ASSERT(m_cb == nullptr);
	ANKI_ASSERT(resources != nullptr);

	ResourcePointer other;
	Bool found = resources->_findLoadedResource(filename, other);

	if(!found)
	{
		// Allocate m_cb
		U len = filename.getLength();
		PtrSize alignment = alignof(ControlBlock);
		m_cb = reinterpret_cast<ControlBlock*>(
			resources->_getAllocator().allocate(
			sizeof(ControlBlock) + len, &alignment));
		resources->_getAllocator().construct(m_cb);

		// Populate the m_cb
		try
		{
			ResourceInitializer init(
				resources->_getAllocator(),
				resources->_getTempAllocator(),
				*resources);

			m_cb->m_resource.load(filename, init);
		}
		catch(const std::exception& e)
		{
			m_cb->m_resources->_getAllocator().deleteInstance(m_cb);	
		}

		m_cb->m_resources = resources;
		std::memcpy(&m_cb->m_uuid[0], &filename[0], len + 1);

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
	
	if(b.m_cb == nullptr)
	{
		auto count = b.m_cb->m_refcount.fetch_add(1);
		ANKI_ASSERT(count > 0);
		(void)count;

		m_cb = b.m_cb;
	}
}

} // end namespace anki

