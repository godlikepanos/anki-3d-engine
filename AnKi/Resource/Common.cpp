// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/Common.h>
#include <AnKi/Resource.h>

namespace anki {

template<typename T>
void ResourcePtrDeleter<T>::operator()(T* ptr)
{
	ResourceManager::getSingleton().freeResource(ptr);
}

#define ANKI_INSTANTIATE_RESOURCE(className) template void ResourcePtrDeleter<className>::operator()(className* ptr);
#include <AnKi/Resource/Resources.def.h>

} // end namespace anki
