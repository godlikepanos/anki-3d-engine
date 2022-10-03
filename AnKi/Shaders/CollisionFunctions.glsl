// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

/// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
Bool testRayTriangle(Vec3 rayOrigin, Vec3 rayDir, Vec3 v0, Vec3 v1, Vec3 v2, Bool backfaceCulling, out F32 t, out F32 u,
					 out F32 v)
{
	const Vec3 v0v1 = v1 - v0;
	const Vec3 v0v2 = v2 - v0;
	const Vec3 pvec = cross(rayDir, v0v2);
	const F32 det = dot(v0v1, pvec);

	if((backfaceCulling && det < kEpsilonf) || abs(det) < kEpsilonf)
	{
		return false;
	}

	const F32 invDet = 1.0 / det;

	const Vec3 tvec = rayOrigin - v0;
	u = dot(tvec, pvec) * invDet;
	if(u < 0.0 || u > 1.0)
	{
		return false;
	}

	const Vec3 qvec = cross(tvec, v0v1);
	v = dot(rayDir, qvec) * invDet;
	if(v < 0.0 || u + v > 1.0)
	{
		return false;
	}

	t = dot(v0v2, qvec) * invDet;

	if(t <= kEpsilonf)
	{
		// This is an addition to the original code. Can't have rays that don't touch the triangle
		return false;
	}

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

/// Ray box intersection by Simon Green
Bool testRayAabb(Vec3 rayOrigin, Vec3 rayDir, Vec3 aabbMin, Vec3 aabbMax, out F32 t0, out F32 t1)
{
	const Vec3 invR = 1.0 / rayDir;
	const Vec3 tbot = invR * (aabbMin - rayOrigin);
	const Vec3 ttop = invR * (aabbMax - rayOrigin);

	const Vec3 tmin = min(ttop, tbot);
	const Vec3 tmax = max(ttop, tbot);

	t0 = max(tmin.x, max(tmin.y, tmin.z));
	t1 = min(tmax.x, min(tmax.y, tmax.z));

	return t0 < t1 && t1 > kEpsilonf;
}

Bool testRayObb(Vec3 rayOrigin, Vec3 rayDir, Vec3 obbExtend, Mat4 obbTransformInv, out F32 t0, out F32 t1)
{
	// Transform ray to OBB space
	const Vec3 rayOriginS = (obbTransformInv * Vec4(rayOrigin, 1.0)).xyz;
	const Vec3 rayDirS = (obbTransformInv * Vec4(rayDir, 0.0)).xyz;

	// Test as AABB
	return testRayAabb(rayOriginS, rayDirS, -obbExtend, obbExtend, t0, t1);
}

/// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
Bool testRaySphere(Vec3 rayOrigin, Vec3 rayDir, Vec3 sphereCenter, F32 sphereRadius, out F32 t0, out F32 t1)
{
	const Vec3 L = sphereCenter - rayOrigin;
	const F32 tca = dot(L, rayDir);
	const F32 d2 = dot(L, L) - tca * tca;
	const F32 radius2 = sphereRadius * sphereRadius;
	const F32 diff = radius2 - d2;
	if(diff < 0.0)
	{
		return false;
	}

	const F32 thc = sqrt(diff);
	t0 = tca - thc;
	t1 = tca + thc;

	if(t0 < 0.0 && t1 < 0.0)
	{
		return false;
	}

	// Swap
	if(t0 > t1)
	{
		const F32 tmp = t0;
		t0 = t1;
		t1 = tmp;
	}

	t0 = max(0.0, t0);
	return true;
}

F32 testPlanePoint(Vec3 planeNormal, F32 planeOffset, Vec3 point)
{
	return dot(planeNormal, point) - planeOffset;
}
