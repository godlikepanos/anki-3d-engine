// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/collision/Tests.h>
#include <anki/collision/Aabb.h>
#include <anki/collision/Sphere.h>
#include <anki/collision/CompoundShape.h>
#include <anki/collision/LineSegment.h>
#include <anki/collision/Plane.h>
#include <anki/collision/Obb.h>
#include <anki/collision/ConvexHullShape.h>
#include <anki/collision/GjkEpa.h>

namespace anki
{

static Bool gjk(const CollisionShape& a, const CollisionShape& b)
{
	Gjk gjk;
	return gjk.intersect(static_cast<const ConvexShape&>(a), static_cast<const ConvexShape&>(b));
}

static Bool test(const Aabb& a, const Aabb& b)
{
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
}

Bool test(const Aabb& aabb, const LineSegment& ls)
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

static Bool test(const Aabb& aabb, const Sphere& s)
{
	const Vec4& c = s.getCenter();

// find the box's closest point to the sphere
#if ANKI_SIMD == ANKI_SIMD_SSE
	__m128 gt = _mm_cmpgt_ps(c.getSimd(), aabb.getMax().getSimd());
	__m128 lt = _mm_cmplt_ps(c.getSimd(), aabb.getMin().getSimd());

	__m128 m = _mm_or_ps(_mm_and_ps(gt, aabb.getMax().getSimd()), _mm_andnot_ps(gt, c.getSimd()));
	__m128 n = _mm_or_ps(_mm_and_ps(lt, aabb.getMin().getSimd()), _mm_andnot_ps(lt, m));

	Vec4 cp(n);
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
	Vec4 sub = c - cp;

	if(sub.getLengthSquared() <= s.getRadiusSquared())
	{
		return true;
	}

	return false;
}

static Bool test(const LineSegment& ls, const Obb& obb)
{
	F32 maxS = MIN_F32;
	F32 minT = MAX_F32;

	// compute difference vector
	Vec4 diff = obb.getCenter() - ls.getOrigin();

	// for each axis do
	for(U i = 0; i < 3; ++i)
	{
		// get axis i
		Vec4 axis = obb.getRotation().getColumn(i).xyz0();

		// project relative vector onto axis
		F32 e = axis.dot(diff);
		F32 f = ls.getDirection().dot(axis);

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

Bool test(const LineSegment& ls, const Sphere& s)
{
	const Vec4& v = ls.getDirection();
	Vec4 w0 = s.getCenter() - ls.getOrigin();
	F32 w0dv = w0.dot(v);
	F32 rsq = s.getRadius() * s.getRadius();

	if(w0dv < 0.0) // if the ang is >90
	{
		return w0.getLengthSquared() <= rsq;
	}

	Vec4 w1 = w0 - v; // aka center - P1, where P1 = seg.origin + seg.dir
	F32 w1dv = w1.dot(v);

	if(w1dv > 0.0) // if the ang is <90
	{
		return w1.getLengthSquared() <= rsq;
	}

	// the big parenthesis is the projection of w0 to v
	Vec4 tmp = w0 - (v * (w0.dot(v) / v.getLengthSquared()));
	return tmp.getLengthSquared() <= rsq;
}

static Bool test(const Sphere& a, const Sphere& b)
{
	F32 tmp = a.getRadius() + b.getRadius();
	return (a.getCenter() - b.getCenter()).getLengthSquared() <= tmp * tmp;
}

template<typename A, typename B>
static Bool t(const CollisionShape& a, const CollisionShape& b)
{
	return test(static_cast<const A&>(a), static_cast<const B&>(b));
}

template<typename A, typename B>
static Bool tr(const CollisionShape& a, const CollisionShape& b)
{
	return test(static_cast<const B&>(b), static_cast<const A&>(a));
}

/// Test plane.
template<typename A>
static Bool txp(const CollisionShape& a, const CollisionShape& b)
{
	return static_cast<const A&>(a).testPlane(static_cast<const Plane&>(b));
}

/// Test plane.
template<typename A>
static Bool tpx(const CollisionShape& a, const CollisionShape& b)
{
	return txp<A>(b, a);
}

/// Compound shape.
static Bool tcx(const CollisionShape& a, const CollisionShape& b)
{
	Bool inside = false;
	ANKI_ASSERT(a.getType() == CollisionShapeType::COMPOUND);
	const CompoundShape& c = static_cast<const CompoundShape&>(a);

	// Use the error to stop the loop
	Error err = c.iterateShapes([&](const CollisionShape& cs) {
		if(testCollisionShapes(cs, b))
		{
			inside = true;
			return ErrorCode::FUNCTION_FAILED;
		}

		return ErrorCode::NONE;
	});
	(void)err;

	return inside;
}

/// Compound shape.
static Bool txc(const CollisionShape& a, const CollisionShape& b)
{
	return tcx(b, a);
}

using Callback = Bool (*)(const CollisionShape& a, const CollisionShape& b);

static const U COUNT = U(CollisionShapeType::COUNT);

// clang-format off
static const Callback matrix[COUNT][COUNT] = {
/*          P                     LS                      Comp  AABB                  S                        OBB                   CH */
/* P    */ {nullptr,              txp<LineSegment>,       tcx,  txp<Aabb>,            txp<Sphere>,             txp<Obb>,             txp<ConvexHullShape>},
/* LS   */ {tpx<LineSegment>,     nullptr,                tcx,  t<Aabb, LineSegment>, tr<Sphere, LineSegment>, tr<Obb, LineSegment>, nullptr             },
/* Comp */ {tpx<CompoundShape>,   txc,                    tcx,  txc,                  txc,                     txc,                  txc                 },
/* AABB */ {tpx<Aabb>,            tr<LineSegment, Aabb>,  tcx,  t<Aabb, Aabb>,        tr<Sphere, Aabb>,        gjk,                  gjk                 },
/* S    */ {tpx<Sphere>,          t<LineSegment, Sphere>, tcx,  t<Aabb, Sphere>,      t<Sphere, Sphere>,       gjk,                  gjk                 },
/* OBB  */ {tpx<Obb>,             t<LineSegment, Obb>,    tcx,  gjk,                  gjk,                     gjk,                  gjk                 },
/* CH   */ {tpx<ConvexHullShape>, nullptr,                tcx,  gjk,                  gjk,                     gjk,                  gjk                 }};
// clang-format on

Bool testCollisionShapes(const CollisionShape& a, const CollisionShape& b)
{
	U ai = static_cast<U>(a.getType());
	U bi = static_cast<U>(b.getType());

	ANKI_ASSERT(matrix[ai][bi] != nullptr && "Collision algorithm is missing");
	ANKI_ASSERT(ai < COUNT && bi < COUNT && "Out of range");

	return matrix[bi][ai](a, b);
}

} // end namespace anki
