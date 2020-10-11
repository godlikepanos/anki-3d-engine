// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/collision/Common.h>
#include <anki/collision/Plane.h>
#include <anki/collision/Ray.h>
#include <anki/collision/Aabb.h>

namespace anki
{

/// @addtogroup collision
/// @{

/// It gives the distance between a point and a plane. It returns >0 if the primitive lies in front of the plane, <0
/// if it's behind and ==0 when it collides the plane.
F32 testPlane(const Plane& plane, const Aabb& aabb);

/// @copydoc testPlane(const Plane&, const Aabb&)
F32 testPlane(const Plane& plane, const Sphere& sphere);

/// @copydoc testPlane(const Plane&, const Aabb&)
F32 testPlane(const Plane& plane, const Obb& obb);

/// @copydoc testPlane(const Plane&, const Aabb&)
F32 testPlane(const Plane& plane, const ConvexHullShape& hull);

/// @copydoc testPlane(const Plane&, const Aabb&)
F32 testPlane(const Plane& plane, const LineSegment& ls);

/// @copydoc testPlane(const Plane&, const Aabb&)
F32 testPlane(const Plane& plane, const Cone& cone);

/// @copydoc testPlane(const Plane&, const Aabb&)
F32 testPlane(const Plane& plane, const Ray& ray);

/// @copydoc testPlane(const Plane&, const Aabb&)
inline F32 testPlane(const Plane& plane, const Vec4& point)
{
	ANKI_ASSERT(isZero(point.w()));
	return plane.getNormal().dot(point) - plane.getOffset();
}

/// @copydoc computeAabb(const ConvexHullShape&)
Aabb computeAabb(const Sphere& sphere);

/// @copydoc computeAabb(const ConvexHullShape&)
Aabb computeAabb(const Obb& obb);

/// Compute a bounding box of a shape.
Aabb computeAabb(const ConvexHullShape& hull);

/// @copydoc computeAabb(const ConvexHullShape&)
Aabb computeAabb(const LineSegment& ls);

/// @copydoc computeAabb(const ConvexHullShape&)
Aabb computeAabb(const Cone& cone);

// Aabb
Bool testCollision(const Aabb& a, const Aabb& b);
Bool testCollision(const Aabb& a, const Sphere& b);
Bool testCollision(const Aabb& a, const Obb& b);
Bool testCollision(const Aabb& a, const ConvexHullShape& b);
Bool testCollision(const Aabb& a, const LineSegment& b);
Bool testCollision(const Aabb& a, const Cone& b);
Bool testCollision(const Aabb& a, const Ray& b);

// Sphere
inline Bool testCollision(const Sphere& a, const Aabb& b)
{
	return testCollision(b, a);
}
Bool testCollision(const Sphere& a, const Sphere& b);
Bool testCollision(const Sphere& a, const Obb& b);
Bool testCollision(const Sphere& a, const ConvexHullShape& b);
Bool testCollision(const Sphere& a, const LineSegment& b);
Bool testCollision(const Sphere& a, const Cone& b);
Bool testCollision(const Sphere& a, const Ray& b);

// Obb
inline Bool testCollision(const Obb& a, const Aabb& b)
{
	return testCollision(b, a);
}
inline Bool testCollision(const Obb& a, const Sphere& b)
{
	return testCollision(b, a);
}
Bool testCollision(const Obb& a, const Obb& b);
Bool testCollision(const Obb& a, const ConvexHullShape& b);
Bool testCollision(const Obb& a, const LineSegment& b);
Bool testCollision(const Obb& a, const Cone& b);
Bool testCollision(const Obb& a, const Ray& b);

// ConvexHullShape
inline Bool testCollision(const ConvexHullShape& a, const Aabb& b)
{
	return testCollision(b, a);
}
inline Bool testCollision(const ConvexHullShape& a, const Sphere& b)
{
	return testCollision(b, a);
}
inline Bool testCollision(const ConvexHullShape& a, const Obb& b)
{
	return testCollision(b, a);
}
Bool testCollision(const ConvexHullShape& a, const ConvexHullShape& b);
Bool testCollision(const ConvexHullShape& a, const LineSegment& b);
Bool testCollision(const ConvexHullShape& a, const Cone& b);
Bool testCollision(const ConvexHullShape& a, const Ray& b);

// LineSegment
inline Bool testCollision(const LineSegment& a, const Aabb& b)
{
	return testCollision(b, a);
}
inline Bool testCollision(const LineSegment& a, const Sphere& b)
{
	return testCollision(b, a);
}
inline Bool testCollision(const LineSegment& a, const Obb& b)
{
	return testCollision(b, a);
}
inline Bool testCollision(const LineSegment& a, const ConvexHullShape& b)
{
	return testCollision(b, a);
}
Bool testCollision(const LineSegment& a, const LineSegment& b);
Bool testCollision(const LineSegment& a, const Cone& b);
Bool testCollision(const LineSegment& a, const Ray& b);

// Cone
inline Bool testCollision(const Cone& a, const Aabb& b)
{
	return testCollision(b, a);
}
inline Bool testCollision(const Cone& a, const Sphere& b)
{
	return testCollision(b, a);
}
inline Bool testCollision(const Cone& a, const Obb& b)
{
	return testCollision(b, a);
}
inline Bool testCollision(const Cone& a, const ConvexHullShape& b)
{
	return testCollision(b, a);
}
inline Bool testCollision(const Cone& a, const LineSegment& b)
{
	return testCollision(b, a);
}
Bool testCollision(const Cone& a, const Cone& b);
Bool testCollision(const Cone& a, const Ray& b);

// Ray
inline Bool testCollision(const Ray& a, const Aabb& b)
{
	return testCollision(b, a);
}
inline Bool testCollision(const Ray& a, const Sphere& b)
{
	return testCollision(b, a);
}
inline Bool testCollision(const Ray& a, const Obb& b)
{
	return testCollision(b, a);
}
inline Bool testCollision(const Ray& a, const ConvexHullShape& b)
{
	return testCollision(b, a);
}
inline Bool testCollision(const Ray& a, const LineSegment& b)
{
	return testCollision(b, a);
}
inline Bool testCollision(const Ray& a, const Cone& b)
{
	return testCollision(b, a);
}
Bool testCollision(const Cone& a, const Ray& b);

// Extra testCollision functions

Bool testCollision(const Plane& plane, const Ray& ray, Vec4& intersection);
Bool testCollision(const Plane& plane, const Vec4& vector, Vec4& intersection);
Bool testCollision(const Sphere& sphere, const Ray& ray, Array<Vec4, 2>& intersectionPoints, U& intersectionPointCount);

// Intersect a ray against an AABB. The ray is inside the AABB. The function returns the distance 'a' where the
// intersection point is rayOrigin + rayDir * a
// https://community.arm.com/graphics/b/blog/posts/reflections-based-on-local-cubemaps-in-unity
inline F32 testCollisionInside(const Aabb& aabb, const Ray& ray)
{
	const Vec4 reciprocal = ray.getDirection().reciprocal();
	const Vec4 intersectMaxPointPlanes = ((aabb.getMax() - ray.getOrigin()) * reciprocal).xyz0();
	const Vec4 intersectMinPointPlanes = ((aabb.getMin() - ray.getOrigin()) * reciprocal).xyz0();
	const Vec4 largestParams = intersectMaxPointPlanes.max(intersectMinPointPlanes);
	const F32 distToIntersect = min(min(largestParams.x(), largestParams.y()), largestParams.z());
	return distToIntersect;
}

#undef ANKI_DEF_TEST_COLLISION_FUNC
#undef ANKI_DEF_TEST_COLLISION_FUNC_PLANE

/// Get the min distance of a point to a plane.
inline F32 getPlanePointDistance(const Plane& plane, const Vec4& point)
{
	return absolute(testPlane(plane, point));
}

/// Returns the perpedicular point of a given point in a plane. Plane's normal and returned-point are perpedicular.
inline Vec4 projectPointToPlane(const Plane& plane, const Vec4& point)
{
	return point - plane.getNormal() * testPlane(plane, point);
}

/// Extract the clip planes using an MVP matrix.
void extractClipPlanes(const Mat4& mvp, Array<Plane, 6>& planes);

/// See extractClipPlanes.
void extractClipPlane(const Mat4& mvp, FrustumPlaneType id, Plane& plane);

/// Compute the edges of the far plane of a frustum
void computeEdgesOfFrustum(F32 far, F32 fovX, F32 fovY, Vec4 points[4]);

/// @}

} // end namespace anki