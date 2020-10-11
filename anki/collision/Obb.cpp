// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/collision/Obb.h>

namespace anki
{

Obb Obb::getCompoundShape(const Obb& b) const
{
	check();
	b.check();

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

	out.setFromPointCloud(reinterpret_cast<const Vec3*>(&points[0]), points.getSize(), sizeof(Vec4), sizeof(points));
	return out;
}

void Obb::getExtremePoints(Array<Vec4, 8>& points) const
{
	check();

	// L: left, R: right, T: top, B: bottom, F: front, B: back
	enum : U8
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

	const Vec3 er3 = m_rotation * m_extend; // extend rotated
	const Vec4 er(er3, 0.0f);

	points[RTF] = er;
	points[LBB] = -er;

	const Vec4 xAxis = Vec4(m_rotation.getColumn(0).getNormalized(), 0.0f);
	const Vec4 yAxis = Vec4(m_rotation.getColumn(1).getNormalized(), 0.0f);
	const Vec4 zAxis = Vec4(m_rotation.getColumn(2).getNormalized(), 0.0f);

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

void Obb::setFromPointCloud(const Vec3* pointBuffer, U pointCount, PtrSize pointStride, PtrSize buffSize)
{
	// Preconditions
	ANKI_ASSERT(pointBuffer);
	ANKI_ASSERT(pointCount > 1);
	ANKI_ASSERT(pointStride >= sizeof(Vec3));
	ANKI_ASSERT(buffSize >= pointStride * pointCount);

	Vec4 min = Vec4(Vec3(MAX_F32), 0.0);
	Vec4 max = Vec4(Vec3(MIN_F32), 0.0);

	// Iterate
	const U8* ptr = reinterpret_cast<const U8*>(pointBuffer);
	while(pointCount-- != 0)
	{
		ANKI_ASSERT((ptrToNumber(ptr) + sizeof(Vec3) - ptrToNumber(pointBuffer)) <= buffSize);
		const Vec3* pos = reinterpret_cast<const Vec3*>(ptr);

		max = max.max(pos->xyz0());
		min = min.min(pos->xyz0());

		ptr += pointStride;
	}

	// Set the locals
	m_center = (max + min) / 2.0;
	m_rotation = Mat3x4::getIdentity();
	m_extend = max - m_center;
}

Vec4 Obb::computeSupport(const Vec4& dir) const
{
	check();

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
