// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_COLLISION_TESTS_H
#define ANKI_COLLISION_TESTS_H

#include <anki/collision/Common.h>

namespace anki {

/// @addtogroup collision
/// @{

/// Test if two collision shapes collide.
Bool testCollisionShapes(const CollisionShape& a, const CollisionShape& b);
/// @}

} // end namespace anki

#endif

