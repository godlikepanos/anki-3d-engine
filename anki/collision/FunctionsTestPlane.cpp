#include <anki/collision/Functions.h>
#include <anki/collision/ConvexHullShape.h>
#include <anki/collision/Obb.h>
#include <anki/collision/LineSegment.h>
#include <anki/collision/Cone.h>
#include <anki/collision/Sphere.h>

namespace anki
{

F32 testPlane(const Plane& plane, const Aabb& aabb)
{
#if ANKI_SIMD_SSE
	__m128 gezero = _mm_cmpge_ps(plane.getNormal().getSimd(), _mm_setzero_ps());

	Vec4 diagMin;
	diagMin.getSimd() =
		_mm_or_ps(_mm_and_ps(gezero, aabb.getMin().getSimd()), _mm_andnot_ps(gezero, aabb.getMax().getSimd()));
#else
	Vec4 diagMin(0.0f), diagMax(0.0f);
	// set min/max values for x,y,z direction
	for(U i = 0; i < 3; i++)
	{
		if(plane.getNormal()[i] >= 0.0f)
		{
			diagMin[i] = aabb.getMin()[i];
			diagMax[i] = aabb.getMax()[i];
		}
		else
		{
			diagMin[i] = aabb.getMax()[i];
			diagMax[i] = aabb.getMin()[i];
		}
	}
#endif

	// minimum on positive side of plane, box on positive side
	ANKI_ASSERT(diagMin.w() == 0.0f);
	F32 test = testPlane(plane, diagMin);
	if(test > 0.0f)
	{
		return test;
	}

#if ANKI_SIMD_SSE
	Vec4 diagMax;
	diagMax.getSimd() =
		_mm_or_ps(_mm_and_ps(gezero, aabb.getMax().getSimd()), _mm_andnot_ps(gezero, aabb.getMin().getSimd()));
#endif

	ANKI_ASSERT(diagMax.w() == 0.0f);
	test = testPlane(plane, diagMax);
	if(test >= 0.0f)
	{
		// min on non-positive side, max on non-negative side, intersection
		return 0.0f;
	}
	else
	{
		// max on negative side, box on negative side
		return test;
	}
}

F32 testPlane(const Plane& plane, const Sphere& sphere)
{
	const F32 dist = testPlane(plane, sphere.getCenter());
	F32 out = dist - sphere.getRadius();
	if(out > 0.0f)
	{
		// Do nothing
	}
	else
	{
		out = dist + sphere.getRadius();
		if(out < 0.0f)
		{
			// Do nothing
		}
		else
		{
			out = 0.0f;
		}
	}

	return out;
}

F32 testPlane(const Plane& plane, const Obb& obb)
{
	Mat3x4 transposedRotation = obb.getRotation();
	transposedRotation.transposeRotationPart();
	const Vec4 xNormal = (transposedRotation * plane.getNormal()).xyz0();

	// maximum extent in direction of plane normal
	const Vec4 rv = obb.getExtend() * xNormal;
	const Vec4 rvabs = rv.abs();
	const F32 r = rvabs.x() + rvabs.y() + rvabs.z();

	// signed distance between box center and plane
	const F32 d = testPlane(plane, obb.getCenter());

	// return signed distance
	if(absolute(d) < r)
	{
		return 0.0f;
	}
	else if(d < 0.0f)
	{
		return d + r;
	}
	else
	{
		return d - r;
	}
}

F32 testPlane(const Plane& plane, const ConvexHullShape& hull)
{
	// Compute the invert transformation of the plane instead
	const Plane pa = (hull.isTransformIdentity()) ? plane : plane.getTransformed(hull.getInvertTransform());

	F32 minDist = MAX_F32;
	F32 maxDist = MIN_F32;

	ConstWeakArray<Vec4> points = hull.getPoints();
	for(const Vec4& point : points)
	{
		const F32 test = testPlane(pa, point);
		if(ANKI_UNLIKELY(test == 0.0f))
		{
			// Early exit
			return 0.0f;
		}

		minDist = min(minDist, test);
		maxDist = max(maxDist, test);
	}

	if(minDist > 0.0f && maxDist > 0.0f)
	{
		return minDist;
	}
	else if(minDist < 0.0f && maxDist < 0.0f)
	{
		return maxDist;
	}
	else
	{
		return 0.0f;
	}
}

F32 testPlane(const Plane& plane, const LineSegment& ls)
{
	const Vec4& p0 = ls.getOrigin();
	Vec4 p1 = ls.getOrigin() + ls.getDirection();

	const F32 dist0 = testPlane(plane, p0);
	const F32 dist1 = testPlane(plane, p1);

	if(dist0 > 0.0f)
	{
		if(dist1 > 0.0f)
		{
			return min(dist0, dist1);
		}
		else
		{
			return 0.0f;
		}
	}
	else
	{
		if(dist1 < 0.0f)
		{
			return max(dist0, dist1);
		}
		else
		{
			return 0.0f;
		}
	}
}

} // end namespace anki
