#ifndef ANKI_COLLISION_COMMON_H
#define ANKI_COLLISION_COMMON_H

#include "anki/collision/Forward.h"
#include "anki/util/Allocator.h"
#include "anki/util/Vector.h"

namespace anki {

/// @addtogroup collision
/// @{

/// The type of the collision temporary allocator
template<typename T>
using CollisionTempAllocator = StackAllocator<T, false>;

/// A temporary vector
template<typename T>
using CollisionTempVector = Vector<T, CollisionTempAllocator<T>>;

/// @}

} // end namespace anki

#endif

