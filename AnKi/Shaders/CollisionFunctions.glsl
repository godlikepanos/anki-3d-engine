// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

/// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
Bool testRayTriangle(Vec3 orig, Vec3 dir, Vec3 v0, Vec3 v1, Vec3 v2, out F32 t, out F32 u, out F32 v)
{
	const Vec3 v0v1 = v1 - v0;
	const Vec3 v0v2 = v2 - v0;
	const Vec3 pvec = cross(dir, v0v2);
	const F32 det = dot(v0v1, pvec);

	if(det < EPSILON)
	{
		return false;
	}

	const F32 invDet = 1.0 / det;

	const Vec3 tvec = orig - v0;
	u = dot(tvec, pvec) * invDet;
	if(u < 0.0 || u > 1.0)
	{
		return false;
	}

	const Vec3 qvec = cross(tvec, v0v1);
	v = dot(dir, qvec) * invDet;
	if(v < 0.0 || u + v > 1.0)
	{
		return false;
	}

	t = dot(v0v2, qvec) * invDet;
	return true;
}

/// Return true if to AABBs overlap.
Bool testAabbAabb(Vec3 aMin, Vec3 aMax, Vec3 bMin, Vec3 bMax)
{
	return all(lessThan(aMin, bMax)) && all(lessThan(bMin, aMax));
}

/// Intersect a ray against an AABB. The ray is inside the AABB. The function returns the distance 'a' where the
/// intersection point is rayOrigin + rayDir * a
/// https://community.arm.com/graphics/b/blog/posts/reflections-based-on-local-cubemaps-in-unity
F32 testRayAabbInside(Vec3 rayOrigin, Vec3 rayDir, Vec3 aabbMin, Vec3 aabbMax)
{
	const Vec3 intersectMaxPointPlanes = (aabbMax - rayOrigin) / rayDir;
	const Vec3 intersectMinPointPlanes = (aabbMin - rayOrigin) / rayDir;
	const Vec3 largestParams = max(intersectMaxPointPlanes, intersectMinPointPlanes);
	const F32 distToIntersect = min(min(largestParams.x, largestParams.y), largestParams.z);
	return distToIntersect;
}

Bool testRaySphere(Vec3 rayOrigin, Vec3 rayDir, Vec3 sphereCenter, F32 sphereRadius)
{
	const Vec3 L = sphereCenter - rayOrigin;
	const F32 tca = dot(L, rayDir);
	const F32 d2 = dot(L, L) - tca * tca;
	return d2 <= radius * radius;
}
