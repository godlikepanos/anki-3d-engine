// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/Common.h>
#include <AnKi/Resource.h>

namespace anki {

// Instan

template<typename T>
void ResourcePtrDeleter<T>::operator()(T* ptr)
{
	ptr->getManager().unregisterResource(ptr);
	HeapMemoryPool& pool = ptr->getMemoryPool();
	deleteInstance(pool, ptr);
}

#define ANKI_INSTANTIATE_RESOURCE(rsrc_, ptr_) template void ResourcePtrDeleter<rsrc_>::operator()(rsrc_* ptr);

#define ANKI_INSTANSIATE_RESOURCE_DELIMITER()

#include <AnKi/Resource/InstantiationMacros.h>
#undef ANKI_INSTANTIATE_RESOURCE
#undef ANKI_INSTANSIATE_RESOURCE_DELIMITER

} // end namespace anki
