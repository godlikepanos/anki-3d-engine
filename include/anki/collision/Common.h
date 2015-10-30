// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_COLLISION_COMMON_H
#define ANKI_COLLISION_COMMON_H

#include <anki/collision/Forward.h>
#include <anki/util/Allocator.h>

namespace anki {

/// @addtogroup collision
/// @{

/// The type of the collision temporary allocator
template<typename T>
using CollisionTempAllocator = StackAllocator<T>;

template<typename T>
using CollisionAllocator = ChainAllocator<T>;

/// @}

} // end namespace anki

#endif

