// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/collision/Plane.h>

namespace anki
{

void Plane::setFrom3Points(const Vec4& p0, const Vec4& p1, const Vec4& p2)
{
	ANKI_ASSERT(p0.w() == 0.0f && p1.w() == 0.0f && p2.w() == 0.0f);

	// get plane vectors
	const Vec4 u = p1 - p0;
	const Vec4 v = p2 - p0;

	m_normal = u.cross(v);

	// length of normal had better not be zero
	ANKI_ASSERT(m_normal.getLengthSquared() != 0.0f);

	m_normal.normalize();
	m_offset = m_normal.dot(p0);
}

void Plane::setFromPlaneEquation(F32 a, F32 b, F32 c, F32 d)
{
	m_normal = Vec4(a, b, c, 0.0f);

	// length of normal had better not be zero
	ANKI_ASSERT(isZero(m_normal.getLength() - 1.0));

	m_offset = d;
}

Plane Plane::getTransformed(const Transform& trf) const
{
	check();
	Plane plane;

	// Rotate the normal
	plane.m_normal = Vec4(trf.getRotation() * m_normal, 0.0f);

	// the offset
	Mat3x4 rot = trf.getRotation();
	rot.transposeRotationPart();
	Vec4 newTrans(rot * trf.getOrigin(), 0.0f);
	plane.m_offset = m_offset * trf.getScale() + newTrans.dot(m_normal);

	return plane;
}

} // end namespace anki
