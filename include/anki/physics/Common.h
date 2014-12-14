// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_PHYSICS_COMMON_H
#define ANKI_PHYSICS_COMMON_H

#include "anki/util/StdTypes.h"
#include "anki/util/Enum.h"
#include "anki/Math.h"
#include <Newton.h>

namespace anki {

// Forward
class PhysicsWorld;
class PhysicsCollisionShape;

/// @addtogroup physics
/// @{

/// Material types.
enum class PhysicsMaterialBit: U16
{
	NONE = 0,
	STATIC_GEOMETRY = 1 << 0,
	DYNAMIC_GEOMETRY = 1 << 1,
	RAGDOLL = 1 << 2,
	PARTICLES = 1 << 3
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(PhysicsMaterialBit, inline)
/// @}

} // end namespace anki

#endif

