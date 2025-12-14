// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Common.hlsl>

/// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
Bool testRayTriangle(Vec3 rayOrigin, Vec3 rayDir, Vec3 v0, Vec3 v1, Vec3 v2, Bool backfaceCulling, out F32 t, out F32 u, out F32 v)
{
	const Vec3 v0v1 = v1 - v0;
	const Vec3 v0v2 = v2 - v0;
	const Vec3 pvec = cross(rayDir, v0v2);
	const F32 det = dot(v0v1, pvec);

	t = 0.0f;
	u = 0.0f;
	v = 0.0f;

	if((backfaceCulling && det < kEpsilonF32) || abs(det) < kEpsilonF32)
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

	if(t <= kEpsilonF32)
	{
		// This is an addition to the original code. Can't have rays that don't touch the triangle
		return false;
	}

	return true;
}

/// Return true if to AABBs overlap.
Bool aabbAabbOverlap(Vec3 aMin, Vec3 aMax, Vec3 bMin, Vec3 bMax)
{
	return all(aMin < bMax) && all(bMin < aMax);
}

Bool testSphereSphereCollision(Vec3 sphereCenterA, F32 sphereRadiusA, Vec3 sphereCenterB, F32 sphereRadiusB)
{
	const Vec3 vec = sphereCenterA - sphereCenterB;
	const F32 distSquared = dot(vec, vec);
	const F32 maxDist = sphereRadiusA + sphereRadiusB;
	return (distSquared < maxDist * maxDist);
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

	return t0 < t1 && t1 > kEpsilonF32;
}

Bool testRayObb(Vec3 rayOrigin, Vec3 rayDir, Vec3 obbExtend, Mat4 obbTransformInv, out F32 t0, out F32 t1)
{
	// Transform ray to OBB space
	const Vec3 rayOriginS = mul(obbTransformInv, Vec4(rayOrigin, 1.0)).xyz;
	const Vec3 rayDirS = mul(obbTransformInv, Vec4(rayDir, 0.0)).xyz;

	// Test as AABB
	return testRayAabb(rayOriginS, rayDirS, -obbExtend, obbExtend, t0, t1);
}

/// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
Bool testRaySphere(Vec3 rayOrigin, Vec3 rayDir, Vec3 sphereCenter, F32 sphereRadius, out F32 t0, out F32 t1)
{
	t0 = 0.0f;
	t1 = 0.0f;

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

F32 testPlanePoint(Vec3 planeNormal, F32 planeOffset, Vec3 point3d)
{
	return dot(planeNormal, point3d) - planeOffset;
}

F32 testPlaneAabb(Vec3 planeNormal, F32 planeOffset, Vec3 aabbMin, Vec3 aabbMax)
{
	const bool3 ge = planeNormal >= 0.0;
	const Vec3 diagMin = select(aabbMin, aabbMax, ge);
	const Vec3 diagMax = select(aabbMax, aabbMin, ge);

	F32 test = testPlanePoint(planeNormal, planeOffset, diagMin);
	if(test > 0.0)
	{
		return test;
	}

	test = testPlanePoint(planeNormal, planeOffset, diagMax);
	return (test >= 0.0) ? 0.0 : test;
}

F32 testPlaneSphere(Vec3 planeNormal, F32 planeOffset, Vec3 sphereCenter, F32 sphereRadius)
{
	const F32 centerDist = testPlanePoint(planeNormal, planeOffset, sphereCenter);
	F32 dist = centerDist - sphereRadius;
	if(dist >= 0.0f)
	{
		return dist;
	}

	dist = centerDist + sphereRadius;
	return (dist < 0.0f) ? dist : 0.0f;
}

Bool aabbSphereOverlap(Vec3 aabbMin, Vec3 aabbMax, Vec3 sphereCenter, F32 sphereRadius)
{
	Vec3 closestPoint = sphereCenter;

#if 0
	[unroll] for(U32 i = 0; i < 3; ++i)
	{
		if(sphereCenter[i] < aabbMin[i])
		{
			closestPoint[i] = aabbMin[i];
		}
		else if(sphereCenter[i] > aabbMax[i])
		{
			closestPoint[i] = aabbMax[i];
		}
	}
#else
	closestPoint = select(sphereCenter > aabbMax, aabbMax, sphereCenter);
	closestPoint = select(closestPoint < aabbMin, aabbMin, closestPoint);
#endif

	const Vec3 sub = sphereCenter - closestPoint;
	return dot(sub, sub) <= square(sphereRadius);
}

Bool frustumTest(Vec4 frustumPlanes[6], Vec3 sphereCenter, F32 sphereRadius)
{
	F32 minPlaneDistance = testPlanePoint(frustumPlanes[0].xyz, frustumPlanes[0].w, sphereCenter);
	[unroll] for(U32 i = 1; i < 6; ++i)
	{
		const F32 d = testPlanePoint(frustumPlanes[i].xyz, frustumPlanes[i].w, sphereCenter);
		minPlaneDistance = min(minPlaneDistance, d);
	}

	return minPlaneDistance > -sphereRadius;
}

/// Modified version found in https://zeux.io/2023/01/12/approximate-projected-bounds
void projectAabb(Vec3 aabbMin, Vec3 aabbMax, Mat4 viewProjMat, out Vec2 minNdc, out Vec2 maxNdc, out F32 aabbMinDepth)
{
	const Vec4 SX = mul(viewProjMat, Vec4(aabbMax.x - aabbMin.x, 0.0, 0.0, 0.0));
	const Vec4 SY = mul(viewProjMat, Vec4(0.0, aabbMax.y - aabbMin.y, 0.0, 0.0));
	const Vec4 SZ = mul(viewProjMat, Vec4(0.0, 0.0, aabbMax.z - aabbMin.z, 0.0));

	Vec4 aabbEdgesClip[8u];
	aabbEdgesClip[0] = mul(viewProjMat, Vec4(aabbMin.x, aabbMin.y, aabbMin.z, 1.0));
	aabbEdgesClip[1] = aabbEdgesClip[0] + SZ;
	aabbEdgesClip[2] = aabbEdgesClip[0] + SY;
	aabbEdgesClip[3] = aabbEdgesClip[2] + SZ;
	aabbEdgesClip[4] = aabbEdgesClip[0] + SX;
	aabbEdgesClip[5] = aabbEdgesClip[4] + SZ;
	aabbEdgesClip[6] = aabbEdgesClip[4] + SY;
	aabbEdgesClip[7] = aabbEdgesClip[6] + SZ;

	aabbMinDepth = 1.0f;
	minNdc = 1000.0f;
	maxNdc = -1000.0f;
	[unroll] for(U32 i = 0; i < 8; ++i)
	{
		Vec4 p = aabbEdgesClip[i];
		p.xyz /= abs(p.w);

		minNdc = min(minNdc, p.xy);
		maxNdc = max(maxNdc, p.xy);
		aabbMinDepth = min(aabbMinDepth, p.z);
	}

	if(aabbMinDepth < 0.0)
	{
		// Behind the camera so we can't be sure about our calculations
		minNdc = -1.0;
		maxNdc = 1.0;
	}

	aabbMinDepth = saturate(aabbMinDepth);
}

Bool cullHzb(Vec2 aabbMinNdc, Vec2 aabbMaxNdc, F32 aabbMinDepth, Texture2D<Vec4> hzb, SamplerState nearestAnyClampSampler)
{
	Vec2 texSize;
	F32 mipCount;
	hzb.GetDimensions(0, texSize.x, texSize.y, mipCount);

	const Vec2 uva = saturate(ndcToUv(aabbMinNdc));
	const Vec2 uvb = saturate(ndcToUv(aabbMaxNdc));

	const Vec2 minUv = Vec2(uva.x, uvb.y);
	const Vec2 maxUv = Vec2(uvb.x, uva.y);
	const Vec2 sizeXY = (maxUv - minUv) * texSize;
	F32 mip = ceil(log2(max(sizeXY.x, sizeXY.y)));

	// Try to use a more detailed mip if you can
	const F32 levelLower = max(mip - 1.0, 0.0);
	const Vec2 mipSize = texSize / pow(2.0f, levelLower);
	const Vec2 a = floor(minUv * mipSize);
	const Vec2 b = ceil(maxUv * mipSize);
	const Vec2 dims = b - a;

	if(dims.x <= 2.0 && dims.y <= 2.0)
	{
		mip = levelLower;
	}

	// Sample mip
	Vec4 depths;
	depths[0] = hzb.SampleLevel(nearestAnyClampSampler, minUv, mip);
	depths[1] = hzb.SampleLevel(nearestAnyClampSampler, maxUv, mip);
	depths[2] = hzb.SampleLevel(nearestAnyClampSampler, Vec2(minUv.x, maxUv.y), mip);
	depths[3] = hzb.SampleLevel(nearestAnyClampSampler, Vec2(maxUv.x, minUv.y), mip);
	const F32 maxDepth = max4(depths);

	return (aabbMinDepth > maxDepth);
}

/// All cone values in local space.
Bool cullBackfaceMeshlet(Vec3 coneDirection, F32 coneCosHalfAngle, Vec3 coneApex, Mat3x4 worldTransform, Vec3 cameraWorldPos)
{
	const Vec3 apexWSpace = mul(worldTransform, Vec4(coneApex, 1.0f));
	const Vec3 coneAxisWSpace = normalize(mul(worldTransform, Vec4(coneDirection, 0.0f)));

	const Vec3 dir = normalize(apexWSpace - cameraWorldPos);
	return dot(dir, coneAxisWSpace) >= coneCosHalfAngle;
}

F32 distancePointToLineSegment(Vec2 p, Vec2 lineSegmentA, Vec2 lineSegmentB)
{
	const Vec2 ap = p - lineSegmentA;
	const Vec2 ab = lineSegmentB - lineSegmentA;
	const F32 abLenSq = dot(ab, ab);
	if(abLenSq == 0.0f)
	{
		return length(p - lineSegmentA);
	}

	const F32 t = saturate(dot(ap, ab) / abLenSq);
	const Vec2 closestPoint = lineSegmentA + t * ab;
	return length(p - closestPoint);
}

Vec4 computePlane(Vec3 p0, Vec3 p1, Vec3 p2)
{
	const Vec3 u = p1 - p0;
	const Vec3 v = p2 - p0;

	const Vec3 normal = normalize(cross(u, v));
	const F32 offset = dot(normal, p1);

	return Vec4(normal, offset);
}
