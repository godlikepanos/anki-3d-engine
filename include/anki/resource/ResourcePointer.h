// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/resource/ResourceManager.h"
#include "anki/util/Ptr.h"
#include "anki/util/Atomic.h"

namespace anki {

/// @addtogroup resource
/// @{

template<typename T>
class ResourcePointerDeleter
{
public:
	void operator()(T* ptr)
	{
		ptr->getManager().unregisterResource(ptr);
		auto alloc = ptr->getAllocator();
		alloc.deleteInstance(ptr);
	}
};

/// Special smart pointer that points to resource classes.
///
/// It looks like auto_ptr but the main difference is that when its out of scope
/// it tries to unload the resource.
template<typename Type>
class ResourcePointer: public IntrusivePtr<Type, ResourcePointerDeleter<Type>>
{
public:
	using Value = Type; ///< Resource type
	using Base = IntrusivePtr<Type, ResourcePointerDeleter<Type>>;
	using Base::get;
	using Base::isCreated;
	using Base::reset;

	CString getResourceName() const
	{
		return get().getUuid().toCString();
	}

	U32 getReferenceCount() const
	{
		return get().getRefcount().load();
	}

	Bool operator==(const ResourcePointer& b) const
	{
		return get().getUuid() == get().getUuid();
	}

	/// Load the resource using the resource manager
	ANKI_USE_RESULT Error load(
		const CString& filename, ResourceManager* resources);

	template<typename... TArgs>
	ANKI_USE_RESULT Error loadToCache(
		ResourceManager* resources, TArgs&&... args);

	Bool isLoaded() const
	{
		return isCreated();
	}

private:
};
/// @}

} // end namespace anki

#include "anki/resource/ResourcePointer.inl.h"

