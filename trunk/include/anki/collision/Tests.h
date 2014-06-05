// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_COLLISION_TESTS_H
#define ANKI_COLLISION_TESTS_H

#include "anki/collision/Common.h"

namespace anki {
namespace detail {

/// @addtogroup collision
/// @{

/// Provides the collision algorithms that detect collision between various
/// shapes
/// @code
/// +------+------+------+------+------+------+------+
/// |      | LS   | OBB  | P    | S    | AABB |      |
/// +------+------+------+------+------+------+------+
/// | LS   | N/A  | OK   | OK   | OK   | OK   |      |
/// +------+------+------+------+------+------+------+
/// | OBB  |      | OK   | OK   | OK   | OK   |      |
/// +------+------+------+------+------+------+------+
/// | P    |      |      | OK   | OK   | OK   |      |
/// +------+------+------+------+------+------+------+
/// | S    |      |      |      | OK   | OK   |      |
/// +------+------+------+------+------+------+------+
/// | AABB |      |      |      |      | OK   |      |
/// +------+------+------+------+------+------+------+
/// @endcode


// 2nd line
U test(const Obb& a, const Obb& b, CollisionTempVector<ContactPoint>& points);

/// @}

} // end namespace detail 
} // end namespace anki

#endif

