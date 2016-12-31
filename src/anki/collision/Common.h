// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/collision/Forward.h>
#include <anki/util/Allocator.h>

namespace anki
{

/// @addtogroup collision
/// @{

/// The type of the collision temporary allocator
template<typename T>
using CollisionTempAllocator = StackAllocator<T>;

template<typename T>
using CollisionAllocator = ChainAllocator<T>;
/// @}

} // end namespace anki
