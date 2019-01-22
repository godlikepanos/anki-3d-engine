// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
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

//======================================================================================================================

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
inline F32 testPlane(const Plane& plane, const Cone& cone)
{
	ANKI_ASSERT(!"TODO");
	return 0.0f;
}

/// @copydoc testPlane(const Plane&, const Aabb&)
inline F32 testPlane(const Plane& plane, const Ray& ray)
{
	ANKI_ASSERT(!"TODO");
	return 0.0f;
}

/// @copydoc testPlane(const Plane&, const Aabb&)
inline F32 testPlane(const Plane& plane, const Vec4& point)
{
	ANKI_ASSERT(isZero(point.w()));
	return plane.getNormal().dot(point) - plane.getOffset();
}

//======================================================================================================================

/// @copydoc computeAabb(const ConvexHullShape&)
Aabb computeAabb(const Sphere& sphere);

/// @copydoc computeAabb(const ConvexHullShape&)
Aabb computeAabb(const Obb& obb);

/// Compute a bounding box of a shape.
Aabb computeAabb(const ConvexHullShape& hull);

/// @copydoc computeAabb(const ConvexHullShape&)
Aabb computeAabb(const LineSegment& ls);

/// @copydoc computeAabb(const ConvexHullShape&)
Aabb computeAabb(const Cone& cone)
{
	ANKI_ASSERT(!"TODO");
	return Aabb(Vec3(0.0f), Vec3(1.0f));
}

//======================================================================================================================

#define ANKI_DEF_TEST_COLLISION_FUNC(type0, type1) \
	Bool testCollision(const type0&, const type1&); \
	inline Bool testCollision(const type1& arg1, const type0& arg0) \
	{ \
		return testCollision(arg0, arg1); \
	}

#define ANKI_DEF_TEST_COLLISION_FUNC_PLANE(type) \
	inline Bool testCollision(const type& arg0, const Plane& plane) \
	{ \
		return testPlane(plane, arg0) == 0.0f; \
	} \
	inline Bool testCollision(const Plane& plane, const type& arg0) \
	{ \
		return testPlane(plane, arg0) == 0.0f; \
	}

Bool testCollision(const Aabb& a, const Aabb& b);
ANKI_DEF_TEST_COLLISION_FUNC(Aabb, Sphere)
ANKI_DEF_TEST_COLLISION_FUNC(Aabb, Obb)
ANKI_DEF_TEST_COLLISION_FUNC(Aabb, ConvexHullShape)
ANKI_DEF_TEST_COLLISION_FUNC(Aabb, LineSegment)
ANKI_DEF_TEST_COLLISION_FUNC(Aabb, Cone)
ANKI_DEF_TEST_COLLISION_FUNC(Aabb, Ray)
ANKI_DEF_TEST_COLLISION_FUNC_PLANE(Aabb)

Bool testCollision(const Sphere& a, const Sphere& b);
ANKI_DEF_TEST_COLLISION_FUNC(Sphere, Obb)
ANKI_DEF_TEST_COLLISION_FUNC(Sphere, ConvexHullShape)
ANKI_DEF_TEST_COLLISION_FUNC(Sphere, LineSegment)
ANKI_DEF_TEST_COLLISION_FUNC(Sphere, Cone)
ANKI_DEF_TEST_COLLISION_FUNC(Sphere, Ray)
ANKI_DEF_TEST_COLLISION_FUNC_PLANE(Sphere)

Bool testCollision(const Obb& a, const Obb& b);
ANKI_DEF_TEST_COLLISION_FUNC(Obb, ConvexHullShape)
ANKI_DEF_TEST_COLLISION_FUNC(Obb, LineSegment)
ANKI_DEF_TEST_COLLISION_FUNC(Obb, Cone)
ANKI_DEF_TEST_COLLISION_FUNC_PLANE(Obb)

Bool testCollision(const ConvexHullShape& a, const ConvexHullShape& b);
ANKI_DEF_TEST_COLLISION_FUNC(ConvexHullShape, LineSegment)
ANKI_DEF_TEST_COLLISION_FUNC(ConvexHullShape, Cone)
ANKI_DEF_TEST_COLLISION_FUNC(ConvexHullShape, Ray)
ANKI_DEF_TEST_COLLISION_FUNC_PLANE(ConvexHullShape)

Bool testCollision(const LineSegment& a, const LineSegment& b);
ANKI_DEF_TEST_COLLISION_FUNC(LineSegment, Cone)
ANKI_DEF_TEST_COLLISION_FUNC(LineSegment, Ray)
ANKI_DEF_TEST_COLLISION_FUNC_PLANE(LineSegment)

Bool testCollision(const Cone& a, const Cone& b);
ANKI_DEF_TEST_COLLISION_FUNC(Cone, Ray)
ANKI_DEF_TEST_COLLISION_FUNC_PLANE(Cone)

Bool testCollision(const Ray& a, const Ray& b);
ANKI_DEF_TEST_COLLISION_FUNC_PLANE(Ray)

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

//======================================================================================================================

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

/// @}

} // end namespace anki