// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/collision/Obb.h"
#include "anki/collision/Plane.h"
#include "anki/collision/Aabb.h"

namespace anki {

//==============================================================================
Obb::Obb()
	:	Base(Type::OBB),
		m_center(Vec4(0.0)),
		m_rotation(Mat3x4::getIdentity()),
		m_extend(Vec3(getEpsilon<F32>()), 0.0)
{}

//==============================================================================
Obb::Obb(const Obb& b)
	: 	Base(Type::OBB)
{
	operator=(b);
}

//==============================================================================
Obb::Obb(const Vec4& center, const Mat3x4& rotation, const Vec4& extend)
	:	Base(Type::OBB), 
		m_center(center),
		m_rotation(rotation),
		m_extend(extend)
{}

//==============================================================================
F32 Obb::testPlane(const Plane& p) const
{
	Mat3x4 rot = m_rotation;
	rot.transposeRotationPart();
	Vec3 xNormal = rot * p.getNormal();

	// maximum extent in direction of plane normal
	F32 r =
		abs(m_extend.x() * xNormal.x()) +
		abs(m_extend.y() * xNormal.y()) +
		abs(m_extend.z() * xNormal.z());
	// signed distance between box center and plane
	F32 d = p.test(m_center);

	// return signed distance
	if(fabs(d) < r)
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

//==============================================================================
Obb Obb::getTransformed(const Transform& trf) const
{
	Obb out;
	out.m_extend = m_extend * trf.getScale();
	out.m_center = trf.transform(m_center);
	out.m_rotation = trf.getRotation().combineTransformations(m_rotation);
	return out;
}

//==============================================================================
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

	out.setFromPointCloud(&points[0], points.size(), sizeof(Vec4), 
		sizeof(Vec4) * points.size());
	return out;
}

//==============================================================================
void Obb::getExtremePoints(Array<Vec4, 8>& points) const
{
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

	Vec4 xAxis = Vec4(m_rotation.getColumn(0), 0.0);
	Vec4 yAxis = Vec4(m_rotation.getColumn(1), 0.0);
	Vec4 zAxis = Vec4(m_rotation.getColumn(2), 0.0);

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
}

//==============================================================================
void Obb::computeAabb(Aabb& aabb) const
{
	Mat3x4 absM;
	for(U i = 0; i < 12; ++i)
	{
		absM[i] = fabs(m_rotation[i]);
	}

	Vec4 newE = Vec4(absM * m_extend, 0.0);

	// Add a small epsilon to avoid some assertions
	Vec4 epsilon(Vec3(getEpsilon<F32>() * 100.0), 0.0);
	aabb = Aabb(m_center - newE, 
		m_center + newE + epsilon);
}

//==============================================================================
void Obb::setFromPointCloud(
	const void* buff, U count, PtrSize stride, PtrSize buffSize)
{
	Vec4 min = Vec4(Vec3(MAX_F32), 0.0);
	Vec4 max = Vec4(Vec3(MIN_F32), 0.0);

	iteratePointCloud(buff, count, stride, buffSize, [&](const Vec3& pos)
	{
		for(U j = 0; j < 3; j++)
		{
			if(pos[j] > max[j])
			{
				max[j] = pos[j];
			}
			else if(pos[j] < min[j])
			{
				min[j] = pos[j];
			}
		}
	});

	// Set the locals
	m_center = (max + min) / 2.0;
	m_rotation = Mat3x4::getIdentity();
	m_extend = max - m_center;
}

//==============================================================================
Vec4 Obb::computeSupport(const Vec4& dir) const
{
	Vec4 out(0.0);

	Vec3 er3 = m_rotation * m_extend; // extend rotated
	Vec4 er(er3, 0.0);

	Vec4 xAxis = Vec4(m_rotation.getColumn(0), 0.0);
	Vec4 yAxis = Vec4(m_rotation.getColumn(1), 0.0);
	Vec4 zAxis = Vec4(m_rotation.getColumn(2), 0.0);

	if(dir.dot(xAxis) >= 0.0)
	{
		// Right side of the box

		out.x() = er.x();
	}
	else
	{
		out.x() = -er.x();
	}

	if(dir.dot(yAxis) >= 0.0)
	{
		// Top side

		out.y() = er.y();
	}
	else
	{
		out.y() = -er.y();
	}

	if(dir.dot(zAxis) >= 0.0)
	{
		// Front side

		out.z() = er.z();
	}
	else
	{
		out.z() = -er.z();
	}

	return out;
}

} // end namespace anki
