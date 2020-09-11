// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/collision/Forward.h>
#include <anki/Math.h>
#include <anki/util/Enum.h>

namespace anki
{

/// @addtogroup collision
/// @{

/// The 6 frustum planes.
enum class FrustumPlaneType : U8
{
	NEAR,
	FAR,
	LEFT,
	RIGHT,
	TOP,
	BOTTOM,

	COUNT,
	FIRST = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(FrustumPlaneType)

/// Collision shape type.
enum class CollisionShapeType : U8
{
	PLANE,
	LINE_SEGMENT,
	RAY,
	AABB,
	SPHERE,
	OBB,
	CONVEX_HULL,
	CONE,

	COUNT,
	FIRST = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(CollisionShapeType)

/// Frustum type
enum class FrustumType : U8
{
	PERSPECTIVE,
	ORTHOGRAPHIC,

	COUNT,
	FIRST = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(FrustumType)
/// @}

} // end namespace anki
