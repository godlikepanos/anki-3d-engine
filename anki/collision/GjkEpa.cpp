// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE
// Inspired by http://vec3.ca/gjk/implementation/

#include <anki/collision/GjkEpa.h>

namespace anki
{

/// GJK support
class GjkSupport
{
public:
	Vec4 m_v;
	Vec4 m_v0;
	Vec4 m_v1;

	Bool operator==(const GjkSupport& b) const
	{
		return m_v == b.m_v && m_v0 == b.m_v0 && m_v1 == b.m_v1;
	}
};

class GjkContext
{
public:
	const void* m_shape0;
	const void* m_shape1;
	GjkSupportCallback m_shape0Callback;
	GjkSupportCallback m_shape1Callback;

	Array<GjkSupport, 4> m_simplex;
	U32 m_count; ///< Simplex count
	Vec4 m_dir;
};

/// Helper of (axb)xa
static Vec4 crossAba(const Vec4& a, const Vec4& b)
{
	// We need to calculate the (axb)xa but we can use the triple product property ax(bxc) = b(a.c) - c(a.b) to make it
	// faster Vec4 out = b * (a.dot(a)) - a * (a.dot(b));
	return a.cross(b.cross(a));
}

static void support(const GjkContext& ctx, GjkSupport& support)
{
	support.m_v0 = ctx.m_shape0Callback(ctx.m_shape0, ctx.m_dir);
	support.m_v1 = ctx.m_shape1Callback(ctx.m_shape1, -ctx.m_dir);
	support.m_v = support.m_v0 - support.m_v1;
}

static Bool update(GjkContext& ctx, const GjkSupport& a)
{
	if(ctx.m_count == 2)
	{
		Vec4 ao = -a.m_v;

		// Compute the vectors parallel to the edges we'll test
		const Vec4 ab = ctx.m_simplex[1].m_v - a.m_v;
		const Vec4 ac = ctx.m_simplex[2].m_v - a.m_v;

		// Compute the triangle's normal
		const Vec4 abc = ab.cross(ac);

		// Compute a vector within the plane of the triangle, Pointing away from the edge ab
		const Vec4 abp = ab.cross(abc);

		if(abp.dot(ao) > 0.0)
		{
			// The origin lies outside the triangle, near the edge ab
			ctx.m_simplex[2] = ctx.m_simplex[1];
			ctx.m_simplex[1] = a;

			ctx.m_dir = crossAba(ab, ao);

			return false;
		}

		// Perform a similar test for the edge ac
		const Vec4 acp = abc.cross(ac);

		if(acp.dot(ao) > 0.0)
		{
			ctx.m_simplex[1] = a;
			ctx.m_dir = crossAba(ac, ao);

			return false;
		}

		// If we get here, then the origin must be within the triangle, but we care whether it is above or below it, so
		// test
		if(abc.dot(ao) > 0.0)
		{
			ctx.m_simplex[3] = ctx.m_simplex[2];
			ctx.m_simplex[2] = ctx.m_simplex[1];
			ctx.m_simplex[1] = a;

			ctx.m_dir = abc;
		}
		else
		{
			ctx.m_simplex[3] = ctx.m_simplex[1];
			ctx.m_simplex[1] = a;

			ctx.m_dir = -abc;
		}

		ctx.m_count = 3;

		// Again, need a tetrahedron to enclose the origin
		return false;
	}
	else if(ctx.m_count == 3)
	{
		const Vec4 ao = -a.m_v;

		Vec4 ab = ctx.m_simplex[1].m_v - a.m_v;
		Vec4 ac = ctx.m_simplex[2].m_v - a.m_v;

		Vec4 abc = ab.cross(ac);

		Vec4 ad, acd, adb;

		if(abc.dot(ao) > 0.0)
		{
			// In front of triangle ABC
			goto check_face;
		}

		ad = ctx.m_simplex[3].m_v - a.m_v;
		acd = ac.cross(ad);

		if(acd.dot(ao) > 0.0)
		{
			// In front of triangle ACD
			ctx.m_simplex[1] = ctx.m_simplex[2];
			ctx.m_simplex[2] = ctx.m_simplex[3];

			ab = ac;
			ac = ad;

			abc = acd;
			goto check_face;
		}

		adb = ad.cross(ab);

		if(adb.dot(ao) > 0.0)
		{
			// In front of triangle ADB

			ctx.m_simplex[2] = ctx.m_simplex[1];
			ctx.m_simplex[1] = ctx.m_simplex[3];

			ac = ab;
			ab = ad;

			abc = adb;
			goto check_face;
		}

		// Behind all three faces, the origin is in the tetrahedron, we're done
		ctx.m_simplex[0] = a;
		ctx.m_count = 4;
		return true;

	check_face:

		// We have a CCW wound triangle ABC the point is in front of this triangle
		// it is NOT "below" edge BC
		// it is NOT "above" the plane through A that's parallel to BC
		const Vec4 abp = ab.cross(abc);

		if(abp.dot(ao) > 0.0)
		{
			ctx.m_simplex[2] = ctx.m_simplex[1];
			ctx.m_simplex[1] = a;

			ctx.m_dir = crossAba(ab, ao);

			ctx.m_count = 2;
			return false;
		}

		const Vec4 acp = abc.cross(ac);

		if(acp.dot(ao) > 0.0)
		{
			ctx.m_simplex[1] = a;

			ctx.m_dir = crossAba(ac, ao);

			ctx.m_count = 2;
			return false;
		}

		ctx.m_simplex[3] = ctx.m_simplex[2];
		ctx.m_simplex[2] = ctx.m_simplex[1];
		ctx.m_simplex[1] = a;

		ctx.m_dir = abc;
		ctx.m_count = 3;

		return false;
	}

	ANKI_ASSERT(0);
	return true;
}

Bool gjkIntersection(const void* shape0, GjkSupportCallback shape0Callback, const void* shape1,
					 GjkSupportCallback shape1Callback)
{
	ANKI_ASSERT(shape0 && shape0Callback && shape1 && shape1Callback);

	GjkContext ctx;
	ctx.m_shape0 = shape0;
	ctx.m_shape1 = shape1;
	ctx.m_shape0Callback = shape0Callback;
	ctx.m_shape1Callback = shape1Callback;

	// Chose random direction
	ctx.m_dir = Vec4(1.0, 0.0, 0.0, 0.0);

	// Do cases 1, 2
	support(ctx, ctx.m_simplex[2]);
	if(ctx.m_simplex[2].m_v.dot(ctx.m_dir) < 0.0)
	{
		return false;
	}

	ctx.m_dir = -ctx.m_simplex[2].m_v;
	support(ctx, ctx.m_simplex[1]);

	if(ctx.m_simplex[1].m_v.dot(ctx.m_dir) < 0.0)
	{
		return false;
	}

	ctx.m_dir = crossAba(ctx.m_simplex[2].m_v - ctx.m_simplex[1].m_v, -ctx.m_simplex[1].m_v);
	ctx.m_count = 2;

	U iterations = 20;
	while(iterations--)
	{
		GjkSupport a;
		support(ctx, a);

		if(a.m_v.dot(ctx.m_dir) < 0.0)
		{
			return false;
		}

		if(update(ctx, a))
		{
			return true;
		}
	}

	return true;
}

} // end namespace anki
