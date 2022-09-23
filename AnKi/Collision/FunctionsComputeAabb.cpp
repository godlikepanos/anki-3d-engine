// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Collision/Functions.h>
#include <AnKi/Collision/ConvexHullShape.h>
#include <AnKi/Collision/Obb.h>
#include <AnKi/Collision/LineSegment.h>
#include <AnKi/Collision/Cone.h>
#include <AnKi/Collision/Sphere.h>

namespace anki {

Aabb computeAabb(const Sphere& sphere)
{
	Aabb aabb;
	aabb.setMin((sphere.getCenter() - sphere.getRadius()).xyz0());
	aabb.setMax((sphere.getCenter() + sphere.getRadius()).xyz0());
	return aabb;
}

Aabb computeAabb(const Obb& obb)
{
	Mat3x4 absM;
	for(U i = 0; i < 12; ++i)
	{
		absM[i] = absolute(obb.getRotation()[i]);
	}

	const Vec4 newE = Vec4(absM * obb.getExtend(), 0.0f);

	// Add a small epsilon to avoid some assertions
	const Vec4 epsilon(Vec3(kEpsilonf * 100.0f), 0.0f);

	return Aabb(obb.getCenter() - newE, obb.getCenter() + newE + epsilon);
}

Aabb computeAabb(const ConvexHullShape& hull)
{
	Vec4 mina(kMaxF32);
	Vec4 maxa(kMinF32);

	for(const Vec4& point : hull.getPoints())
	{
		const Vec4 o = (hull.isTransformIdentity()) ? point : hull.getTransform().transform(point);
		mina = mina.min(o);
		maxa = maxa.max(o);
	}

	return Aabb(mina.xyz0(), maxa.xyz0());
}

Aabb computeAabb(const LineSegment& ls)
{
	const Vec4 p0 = ls.getOrigin();
	const Vec4 p1 = ls.getOrigin() + ls.getDirection();

	const Vec4 min = p0.min(p1);
	const Vec4 max = p0.max(p1);

	return Aabb(min, max);
}

} // end namespace anki
