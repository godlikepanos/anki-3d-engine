// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_COMMON_H
#define ANKI_RESOURCE_COMMON_H

#include "anki/util/Allocator.h"
#include "anki/util/Vector.h"

namespace anki {

template<typename T>
using ResourceAllocator = HeapAllocator<T>;

template<typename T>
using ResourceVector = Vector<T, ResourceAllocator<T>>;

template<typename T>
using TempResourceAllocator = StackAllocator<T>;

template<typename T>
using TempResourceVector = Vector<T, TempResourceAllocator<T>>;

} // end namespace anki

#endif
