// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/collision/Functions.h>
#include <anki/collision/ConvexHullShape.h>
#include <anki/collision/Obb.h>
#include <anki/collision/LineSegment.h>
#include <anki/collision/Cone.h>
#include <anki/collision/Sphere.h>
#include <anki/collision/GjkEpa.h>

namespace anki
{

template<typename T, typename Y>
static Bool testCollisionGjk(const T& a, const Y& b)
{
	auto callbackA = [](const void* shape, const Vec4& dir) {
		return static_cast<const T*>(shape)->computeSupport(dir);
	};
	auto callbackB = [](const void* shape, const Vec4& dir) {
		return static_cast<const Y*>(shape)->computeSupport(dir);
	};
	return gjkIntersection(&a, callbackA, &b, callbackB);
}

Bool testCollision(const Aabb& a, const Aabb& b)
{
#if ANKI_SIMD_SSE
	const __m128 gt0 = _mm_cmpgt_ps(a.getMin().getSimd(), b.getMax().getSimd());
	const __m128 gt1 = _mm_cmpgt_ps(b.getMin().getSimd(), a.getMax().getSimd());

	const __m128 combined = _mm_or_ps(gt0, gt1);

	const int res = _mm_movemask_ps(combined); // Will set the first bit of each byte of combined

	return res == 0;
#else
	// if separated in x direction
	if(a.getMin().x() > b.getMax().x() || b.getMin().x() > a.getMax().x())
	{
		return false;
	}

	// if separated in y direction
	if(a.getMin().y() > b.getMax().y() || b.getMin().y() > a.getMax().y())
	{
		return false;
	}

	// if separated in z direction
	if(a.getMin().z() > b.getMax().z() || b.getMin().z() > a.getMax().z())
	{
		return false;
	}

	// no separation, must be intersecting
	return true;
#endif
}

Bool testCollision(const Aabb& aabb, const Sphere& s)
{
	const Vec4& c = s.getCenter();

	// Find the box's closest point to the sphere
#if ANKI_SIMD_SSE
	__m128 gt = _mm_cmpgt_ps(c.getSimd(), aabb.getMax().getSimd());
	__m128 lt = _mm_cmplt_ps(c.getSimd(), aabb.getMin().getSimd());

	__m128 m = _mm_or_ps(_mm_and_ps(gt, aabb.getMax().getSimd()), _mm_andnot_ps(gt, c.getSimd()));
	__m128 n = _mm_or_ps(_mm_and_ps(lt, aabb.getMin().getSimd()), _mm_andnot_ps(lt, m));

	const Vec4 cp(n);
#else
	Vec4 cp(c); // Closest Point
	for(U i = 0; i < 3; i++)
	{
		// if the center is greater than the max then the closest point is the max
		if(c[i] > aabb.getMax()[i])
		{
			cp[i] = aabb.getMax()[i];
		}
		else if(c[i] < aabb.getMin()[i]) // relative to the above
		{
			cp[i] = aabb.getMin()[i];
		}
		else
		{
			// the c lies between min and max, do nothing
		}
	}
#endif

	// if the c lies totally inside the box then the sub is the zero, this means that the length is also zero and thus
	// it's always smaller than rsq
	const Vec4 sub = c - cp;

	return (sub.getLengthSquared() <= (s.getRadius() * s.getRadius())) ? true : false;
}

Bool testCollision(const Aabb& aabb, const Obb& obb)
{
	return testCollisionGjk(aabb, obb);
}

Bool testCollision(const Aabb& aabb, const ConvexHullShape& hull)
{
	return testCollisionGjk(aabb, hull);
}

Bool testCollision(const Aabb& aabb, const LineSegment& ls)
{
	F32 maxS = MIN_F32;
	F32 minT = MAX_F32;

	// do tests against three sets of planes
	for(U i = 0; i < 3; ++i)
	{
		// segment is parallel to plane
		if(isZero(ls.getDirection()[i]))
		{
			// segment passes by box
			if(ls.getOrigin()[i] < aabb.getMin()[i] || ls.getOrigin()[i] > aabb.getMax()[i])
			{
				return false;
			}
		}
		else
		{
			// compute intersection parameters and sort
			F32 s = (aabb.getMin()[i] - ls.getOrigin()[i]) / ls.getDirection()[i];
			F32 t = (aabb.getMax()[i] - ls.getOrigin()[i]) / ls.getDirection()[i];
			if(s > t)
			{
				F32 temp = s;
				s = t;
				t = temp;
			}

			// adjust min and max values
			if(s > maxS)
			{
				maxS = s;
			}
			if(t < minT)
			{
				minT = t;
			}

			// check for intersection failure
			if(minT < 0.0 || maxS > 1.0 || maxS > minT)
			{
				return false;
			}
		}
	}

	// done, have intersection
	return true;
}

Bool testCollision(const Aabb& aabb, const Cone& cone)
{
	ANKI_ASSERT(!"TODO");
	return false;
}

Bool testCollision(const Sphere& a, const Sphere& b)
{
	const F32 tmp = a.getRadius() + b.getRadius();
	return (a.getCenter() - b.getCenter()).getLengthSquared() <= tmp * tmp;
}

Bool testCollision(const Sphere& sphere, const Obb& obb)
{
	return testCollisionGjk(sphere, obb);
}

Bool testCollision(const Sphere& sphere, const ConvexHullShape& hull)
{
	return testCollisionGjk(sphere, hull);
}

Bool testCollision(const Sphere& s, const LineSegment& ls)
{
	const Vec4& v = ls.getDirection();
	const Vec4 w0 = s.getCenter() - ls.getOrigin();
	const F32 w0dv = w0.dot(v);
	const F32 rsq = s.getRadius() * s.getRadius();

	if(w0dv < 0.0f) // if the ang is >90
	{
		return w0.getLengthSquared() <= rsq;
	}

	const Vec4 w1 = w0 - v; // aka center - P1, where P1 = seg.origin + seg.dir
	const F32 w1dv = w1.dot(v);

	if(w1dv > 0.0f) // if the ang is <90
	{
		return w1.getLengthSquared() <= rsq;
	}

	// the big parenthesis is the projection of w0 to v
	const Vec4 tmp = w0 - (v * (w0.dot(v) / v.getLengthSquared()));
	return tmp.getLengthSquared() <= rsq;
}

Bool testCollision(const Sphere& sphere, const Cone& cone)
{
	// https://bartwronski.com/2017/04/13/cull-that-cone/
	const F32 coneAngle = cone.getAngle() / 2.0f;
	const Vec4 V = sphere.getCenter() - cone.getOrigin();
	const F32 VlenSq = V.dot(V);
	const F32 V1len = V.dot(cone.getDirection());
	const F32 distanceClosestPoint = cos(coneAngle) * sqrt(VlenSq - V1len * V1len) - V1len * sin(coneAngle);

	const Bool angleCull = distanceClosestPoint > sphere.getRadius();
	const Bool frontCull = V1len > sphere.getRadius() + cone.getLength();
	const Bool backCull = V1len < -sphere.getRadius();
	return !(angleCull || frontCull || backCull);
}

Bool testCollision(const Obb& a, const Obb& b)
{
	return testCollisionGjk(a, b);
}

Bool testCollision(const Obb& obb, const ConvexHullShape& hull)
{
	return testCollisionGjk(obb, hull);
}

Bool testCollision(const Obb& obb, const LineSegment& ls)
{
	F32 maxS = MIN_F32;
	F32 minT = MAX_F32;

	// compute difference vector
	const Vec4 diff = obb.getCenter() - ls.getOrigin();

	// for each axis do
	for(U i = 0; i < 3; ++i)
	{
		// get axis i
		const Vec4 axis = obb.getRotation().getColumn(i).xyz0();

		// project relative vector onto axis
		const F32 e = axis.dot(diff);
		const F32 f = ls.getDirection().dot(axis);

		// ray is parallel to plane
		if(isZero(f))
		{
			// ray passes by box
			if(-e - obb.getExtend()[i] > 0.0 || -e + obb.getExtend()[i] > 0.0)
			{
				return false;
			}
			continue;
		}

		F32 s = (e - obb.getExtend()[i]) / f;
		F32 t = (e + obb.getExtend()[i]) / f;

		// fix order
		if(s > t)
		{
			F32 temp = s;
			s = t;
			t = temp;
		}

		// adjust min and max values
		if(s > maxS)
		{
			maxS = s;
		}
		if(t < minT)
		{
			minT = t;
		}

		// check for intersection failure
		if(minT < 0.0 || maxS > 1.0 || maxS > minT)
		{
			return false;
		}
	}

	// done, have intersection
	return true;
}

Bool testCollision(const Obb& obb, const Cone& cone)
{
	ANKI_ASSERT(!"TODO");
	return false;
}

Bool testCollision(const ConvexHullShape& a, const ConvexHullShape& b)
{
	return testCollisionGjk(a, b);
}

Bool testCollision(const ConvexHullShape& hull, const LineSegment& ls)
{
	ANKI_ASSERT(!"TODO");
	return false;
}

Bool testCollision(const ConvexHullShape& hull, const Cone& cone)
{
	ANKI_ASSERT(!"TODO");
	return false;
}

Bool testCollision(const LineSegment& a, const LineSegment& b)
{
	ANKI_ASSERT(!"TODO");
	return false;
}

Bool testCollision(const Cone& a, const Cone& b)
{
	ANKI_ASSERT(!"TODO");
	return false;
}

Bool testCollision(const Plane& plane, const Ray& ray, Vec4& intersection)
{
	Bool intersects = false;

	const F32 d = testPlane(plane, ray.getOrigin()); // Dist of origin to the plane
	const F32 a = plane.getNormal().dot(ray.getDirection());

	if(d > 0.0f && a < 0.0f)
	{
		// To have intersection the d should be positive and the s as well. So the 'a' must be negative and not zero
		// because of the division.
		const F32 s = -d / a;
		ANKI_ASSERT(s > 0.0f);

		intersection = ray.getOrigin() + s * ray.getDirection();
		intersects = true;
	}

	return intersects;
}

Bool testCollision(const Plane& plane, const Vec4& vector, Vec4& intersection)
{
	ANKI_ASSERT(vector.w() == 0.0f);
	const Vec4 pp = vector.getNormalized();
	const F32 dot = pp.dot(plane.getNormal());

	if(!isZero(dot))
	{
		const F32 s = plane.getOffset() / dot;
		intersection = pp * s;
		return true;
	}
	else
	{
		return false;
	}
}

Bool intersect(const Sphere& sphere, const Ray& ray, Array<Vec4, 2>& intersectionPoints, U& intersectionPointCount)
{
	ANKI_ASSERT(isZero(ray.getDirection().getLengthSquared() - 1.0f));

	// See https://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection

	const Vec4& o = ray.getOrigin();
	const Vec4& l = ray.getDirection();
	const Vec4& c = sphere.getCenter();
	const F32 R2 = sphere.getRadius() * sphere.getRadius();

	const Vec4 o_c = o - c;

	const F32 a = l.dot(o_c);
	const F32 b = a * a - o_c.getLengthSquared() + R2;

	if(b < 0.0f)
	{
		intersectionPointCount = 0;
		return false;
	}
	else if(b == 0.0f)
	{
		intersectionPointCount = 1;
		intersectionPoints[0] = -a * l + o;
		return true;
	}
	else
	{
		F32 d = -a - sqrt(b);
		intersectionPointCount = 0;
		if(d > 0.0f)
		{
			intersectionPointCount = 1;
			intersectionPoints[0] = d * l + o;
		}

		d = -a + sqrt(b);
		if(d > 0.0f)
		{
			intersectionPoints[intersectionPointCount++] = d * l + o;
		}

		return intersectionPointCount > 0;
	}
}

} // end namespace anki
