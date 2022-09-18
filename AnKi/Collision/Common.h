// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Collision/Forward.h>
#include <AnKi/Math.h>
#include <AnKi/Util/Enum.h>

namespace anki {

/// @addtogroup collision
/// @{

/// The 6 frustum planes.
enum class FrustumPlaneType : U8
{
	kNear,
	kFar,
	kLeft,
	kRight,
	kTop,
	kBottom,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(FrustumPlaneType)

/// Collision shape type.
enum class CollisionShapeType : U8
{
	kPlane,
	kLineSegment,
	kRay,
	kAABB,
	kSphere,
	kOBB,
	kConvexHull,
	kCone,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(CollisionShapeType)

/// Frustum type
enum class FrustumType : U8
{
	kPerspective,
	kOrthographic,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(FrustumType)
/// @}

} // end namespace anki
