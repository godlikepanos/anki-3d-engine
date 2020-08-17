// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/Common.h>
#include <anki/Resource.h>

namespace anki
{

// Instan

template<typename T>
void ResourcePtrDeleter<T>::operator()(T* ptr)
{
	ptr->getManager().unregisterResource(ptr);
	auto alloc = ptr->getAllocator();
	alloc.deleteInstance(ptr);
}

#define ANKI_INSTANTIATE_RESOURCE(rsrc_, ptr_) template void ResourcePtrDeleter<rsrc_>::operator()(rsrc_* ptr);

#define ANKI_INSTANSIATE_RESOURCE_DELIMITER()

#include <anki/resource/InstantiationMacros.h>
#undef ANKI_INSTANTIATE_RESOURCE
#undef ANKI_INSTANSIATE_RESOURCE_DELIMITER

} // end namespace anki
