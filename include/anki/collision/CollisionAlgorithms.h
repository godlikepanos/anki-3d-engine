// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_COLLISION_COLLISION_ALGORITHMS_H
#define ANKI_COLLISION_COLLISION_ALGORITHMS_H

#include "anki/collision/Forward.h"
#include "anki/util/StdTypes.h"

namespace anki {
namespace detail {

/// @addtogroup Collision
/// @{
/// @addtogroup Algorithms
/// Provides the collision algorithms that detect collision between various
/// shapes
/// @code
/// +------+------+------+------+------+------+------+
/// |      | LS   | OBB  | P    | R    | S    | AABB |
/// +------+------+------+------+------+------+------+
/// | LS   | N/A  | OK   | OK   | N/A  | OK   | OK   |
/// +------+------+------+------+------+------+------+
/// | OBB  |      | OK   | OK   | OK   | OK   | OK   |
/// +------+------+------+------+------+------+------+
/// | P    |      |      | OK   | OK   | OK   | OK   |
/// +------+------+------+------+------+------+------+
/// | R    |      |      |      | N/A  | OK   | OK   |
/// +------+------+------+------+------+------+------+
/// | S    |      |      |      |      | OK   | OK   |
/// +------+------+------+------+------+------+------+
/// | AABB |      |      |      |      |      | OK   |
/// +------+------+------+------+------+------+------+
/// @endcode
/// @{

/// Generic collide function. It doesn't uses visitor pattern for
/// speed reasons
extern Bool collide(const CollisionShape& a, const CollisionShape& b);

// 1st line (LS)
extern Bool collide(const LineSegment& a, const LineSegment& b);
extern Bool collide(const LineSegment& a, const Obb& b);
extern Bool collide(const LineSegment& a, const Plane& b);
extern Bool collide(const LineSegment& a, const Ray& b);
extern Bool collide(const LineSegment& a, const Sphere& b);
extern Bool collide(const LineSegment& a, const Aabb& b);

// 2nd line (OBB)
inline Bool collide(const Obb& a, const LineSegment& b)
{
	return collide(b, a);
}
extern Bool collide(const Obb& a, const Obb& b);
extern Bool collide(const Obb& a, const Plane& b);
extern Bool collide(const Obb& a, const Ray& b);
extern Bool collide(const Obb& a, const Sphere& b);
extern Bool collide(const Obb& a, const Aabb& b);

// 4th line (P)
inline Bool collide(const Plane& a, const LineSegment& b)
{
	return collide(b, a);
}
inline Bool collide(const Plane& a, const Obb& b)
{
	return collide(b, a);
}
extern Bool collide(const Plane& a, const Plane& b);
extern Bool collide(const Plane& a, const Ray& b);
extern Bool collide(const Plane& a, const Sphere& b);
extern Bool collide(const Plane& a, const Aabb& b);

// 5th line (R)
inline Bool collide(const Ray& a, const LineSegment& b)
{
	return collide(b, a);
}
inline Bool collide(const Ray& a, const Obb& b)
{
	return collide(b, a);
}
inline Bool collide(const Ray& a, const Plane& b)
{
	return collide(b, a);
}
extern Bool collide(const Ray& a, const Ray& b);
extern Bool collide(const Ray& a, const Sphere& b);
extern Bool collide(const Ray& a, const Aabb& b);

// 6th line (S)
inline Bool collide(const Sphere& a, const LineSegment& b)
{
	return collide(b, a);
}
inline Bool collide(const Sphere& a, const Obb& b)
{
	return collide(b, a);
}
inline Bool collide(const Sphere& a, const Plane& b)
{
	return collide(b, a);
}
inline Bool collide(const Sphere& a, const Ray& b)
{
	return collide(b, a);
}
extern Bool collide(const Sphere& a, const Sphere& b);
extern Bool collide(const Sphere& a, const Aabb& b);

// 7th line (AABB)
inline Bool collide(const Aabb& a, const LineSegment& b)
{
	return collide(b, a);
}
inline Bool collide(const Aabb& a, const Obb& b)
{
	return collide(b, a);
}
inline Bool collide(const Aabb& a, const Plane& b)
{
	return collide(b, a);
}
inline Bool collide(const Aabb& a, const Ray& b)
{
	return collide(b, a);
}
inline Bool collide(const Aabb& a, const Sphere& b)
{
	return collide(b, a);
}
extern Bool collide(const Aabb& a, const Aabb& b);

/// @}
/// @}

} // end namespace detail
} // end namespace anki

#endif
