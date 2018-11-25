// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/collision/Obb.h>
#include <anki/collision/Plane.h>
#include <anki/collision/Aabb.h>

namespace anki
{

F32 Obb::testPlane(const Plane& p) const
{
	Vec4 xNormal = (m_transposedRotation * p.getNormal()).xyz0();

	// maximum extent in direction of plane normal
	Vec4 rv = m_extend * xNormal;
	Vec4 rvabs = rv.getAbs();
	F32 r = rvabs.x() + rvabs.y() + rvabs.z();

	// signed distance between box center and plane
	F32 d = p.test(m_center);

	// return signed distance
	if(absolute(d) < r)
	{
		return 0.0;
	}
	else if(d < 0.0)
	{
		return d + r;
	}
	else
	{
		return d - r;
	}
}

Obb Obb::getTransformed(const Transform& trf) const
{
	Obb out;
	out.m_extend = m_extend * trf.getScale();
	out.m_center = trf.transform(m_center);
	out.m_rotation = trf.getRotation().combineTransformations(m_rotation);
	out.m_transposedRotation = out.m_rotation;
	out.m_transposedRotation.transposeRotationPart();
	return out;
}

Obb Obb::getCompoundShape(const Obb& b) const
{
	Obb out;

	Array<Vec4, 8> points0;
	Array<Vec4, 8> points1;

	getExtremePoints(points0);
	b.getExtremePoints(points1);

	Array<Vec4, 16> points;
	for(U i = 0; i < 8; i++)
	{
		points[i] = points0[i];
		points[i + 8] = points1[i];
	}

	out.setFromPointCloud(&points[0], points.size(), sizeof(Vec4), sizeof(Vec4) * points.size());
	return out;
}

void Obb::getExtremePoints(Array<Vec4, 8>& points) const
{
	if(!m_cache.m_dirtyExtremePoints)
	{
		points = m_cache.m_extremePoints;
		return;
	}

	m_cache.m_dirtyExtremePoints = false;

	// L: left, R: right, T: top, B: bottom, F: front, B: back
	enum
	{
		RTF,
		LTF,
		LBF,
		RBF,
		RTB,
		LTB,
		LBB,
		RBB
	};

	Vec3 er3 = m_rotation * m_extend; // extend rotated
	Vec4 er(er3, 0.0);

	points[RTF] = er;
	points[LBB] = -er;

	Vec4 xAxis = Vec4(m_rotation.getColumn(0).getNormalized(), 0.0);
	Vec4 yAxis = Vec4(m_rotation.getColumn(1).getNormalized(), 0.0);
	Vec4 zAxis = Vec4(m_rotation.getColumn(2).getNormalized(), 0.0);

	// Reflection: x1' = 2n|x1.n| - x1

	points[RBB] = 2.0f * er.dot(xAxis) * xAxis - er;
	points[LTB] = 2.0f * er.dot(yAxis) * yAxis - er;
	points[LBF] = 2.0f * er.dot(zAxis) * zAxis - er;

	points[LTF] = 2.0f * points[LBB].dot(-xAxis) * -xAxis - points[LBB];
	points[RTB] = 2.0f * points[LTF].dot(yAxis) * yAxis - points[LTF];
	points[RBF] = 2.0f * points[LTF].dot(zAxis) * zAxis - points[LTF];

	for(Vec4& point : points)
	{
		point += m_center;
	}

	// Update cache
	m_cache.m_extremePoints = points;
}

void Obb::computeAabb(Aabb& aabb) const
{
	// Check cache
	if(!m_cache.m_dirtyAabb)
	{
		aabb = m_cache.m_aabb;
		return;
	}
	m_cache.m_dirtyAabb = false;

	Mat3x4 absM;
	for(U i = 0; i < 12; ++i)
	{
		absM[i] = absolute(m_rotation[i]);
	}

	Vec4 newE = Vec4(absM * m_extend, 0.0);

	// Add a small epsilon to avoid some assertions
	Vec4 epsilon(Vec3(EPSILON * 100.0), 0.0);
	aabb = Aabb(m_center - newE, m_center + newE + epsilon);

	// Update cache
	m_cache.m_aabb = aabb;
}

void Obb::setFromPointCloud(const void* buff, U count, PtrSize stride, PtrSize buffSize)
{
	Vec4 min = Vec4(Vec3(MAX_F32), 0.0);
	Vec4 max = Vec4(Vec3(MIN_F32), 0.0);

	iteratePointCloud(buff, count, stride, buffSize, [&](const Vec3& pos) {
		max = max.max(pos.xyz0());
		min = min.min(pos.xyz0());
	});

	// Set the locals
	m_center = (max + min) / 2.0;
	m_rotation = Mat3x4::getIdentity();
	m_transposedRotation = m_rotation;
	m_extend = max - m_center;

	// Invalidate cache
	makeDirty();
}

Vec4 Obb::computeSupport(const Vec4& dir) const
{
	Array<Vec4, 8> points;
	getExtremePoints(points);

	U best = 0;
	F32 bestDot = points[0].dot(dir);

	for(U i = 1; i < points.size(); i++)
	{
		F32 d = points[i].dot(dir);
		if(d > bestDot)
		{
			best = i;
			bestDot = d;
		}
	}

	return points[best];
}

} // end namespace anki
