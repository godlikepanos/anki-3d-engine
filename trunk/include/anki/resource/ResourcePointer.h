// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_RESOURCE_POINTER_H
#define ANKI_RESOURCE_RESOURCE_POINTER_H

#include "anki/util/Assert.h"
#include <atomic>
#include <cstring>

namespace anki {

// Forward
class ResourceManager;

/// @addtogroup resource
/// @{

/// Special smart pointer that points to resource classes.
///
/// It looks like auto_ptr but the main difference is that when its out of scope
/// it tries to unload the resource.
template<typename Type, typename TResourceManager>
class ResourcePointer
{
public:
	using Value = Type; ///< Resource type

	/// Default constructor
	ResourcePointer()
	{}

	/// Copy constructor
	ResourcePointer(const ResourcePointer& b)
	{
		copy(b);
	}

	/// Move contructor
	ResourcePointer(ResourcePointer&& b)
	{
		*this = std::move(b);
	}

	/// Construct and load
	ResourcePointer(const CString& filename)
	{
		load(filename);
	}

	~ResourcePointer()
	{
		reset();
	}

	const Value& operator*() const
	{
		ANKI_ASSERT(m_cb != nullptr);
		return m_cb->m_resource;
	}

	Value& operator*()
	{
		ANKI_ASSERT(m_cb != nullptr);
		return m_cb->m_resource;
	}

	const Value* operator->() const
	{
		ANKI_ASSERT(m_cb != nullptr);
		return &m_cb->m_resource;
	}

	Value* operator->()
	{
		ANKI_ASSERT(m_cb != nullptr);
		return &m_cb->m_resource;
	}

	const Value* get() const
	{
		ANKI_ASSERT(m_cb != nullptr);
		return &m_cb->m_resource;
	}

	Value* get()
	{
		ANKI_ASSERT(m_cb != nullptr);
		return &m_cb->m_resource;
	}

	const char* getResourceName() const
	{
		ANKI_ASSERT(m_cb != nullptr);
		return &m_cb->m_uuid[0];
	}

	U32 getReferenceCount() const
	{
		ANKI_ASSERT(m_cb != nullptr);
		return &m_cb->m_refcount.load();
	}
	
	/// Copy
	ResourcePointer& operator=(const ResourcePointer& b)
	{
		copy(b);
		return *this;
	}

	/// Move
	ResourcePointer& operator=(ResourcePointer&& b)
	{
		m_cb = b.m_cb;
		b.m_cb = nullptr;
		return *this;
	}

	Bool operator==(const ResourcePointer& b) const
	{
		ANKI_ASSERT(m_cb != nullptr);
		ANKI_ASSERT(b.m_cb != nullptr);
		return std::strcmp(&m_cb->m_uuid[0], &b.m_cb->m_uuid[0]) == 0; 
	}

	/// Load the resource using the resource manager
	void load(const char* filename, TResourceManager* resources);

	Bool isLoaded() const
	{
		return m_cb != nullptr;
	}

private:
	/// Control block
	class ControlBlock
	{
	public:
		Type m_resource;
		std::atomic<U32> m_refcount = {1};
		TResourceManager* m_resources = nullptr;
		char m_uuid[1]; ///< This is part of the UUID
	};

	ControlBlock* m_cb = nullptr;

	void reset();

	/// If this empty and @a b empty then unload. If @a b has something then
	/// unload this and load exactly what @b has. In everything else do nothing
	void copy(const ResourcePointer& b);
};

/// @}

} // end namespace anki

#include "anki/resource/ResourcePointer.inl.h"

#endif
