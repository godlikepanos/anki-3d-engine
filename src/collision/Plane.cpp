// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/collision/Plane.h"
#include "anki/util/Assert.h"

namespace anki {

//==============================================================================
Plane::Plane(const Vec4& normal, F32 offset)
	: CollisionShape(Type::PLANE), m_normal(normal), m_offset(offset)
{}

//==============================================================================
F32 Plane::testPlane(const Plane& /*p*/) const
{
	ANKI_ASSERT(0 && "Ambiguous call");
	return 0.0;
}

//==============================================================================
void Plane::setFrom3Points(const Vec3& p0, const Vec3& p1, const Vec3& p2)
{
	// get plane vectors
	Vec4 u = Vec4(p1 - p0, 0.0);
	Vec4 v = Vec4(p2 - p0, 0.0);

	m_normal = u.cross(v);

	// length of normal had better not be zero
	ANKI_ASSERT(!isZero(m_normal.getLengthSquared()));

	m_normal.normalize();
	m_offset = m_normal.dot(Vec4(p0, 0.0)); // XXX: correct??
}

//==============================================================================
void Plane::setFromPlaneEquation(F32 a, F32 b, F32 c, F32 d)
{
	m_normal = Vec4(a, b, c, 0.0);

	// length of normal had better not be zero
	ANKI_ASSERT(isZero(m_normal.getLength() - 1.0));

	m_offset = d;
}

//==============================================================================
Plane Plane::getTransformed(const Transform& trf) const
{
	Plane plane;

	// Rotate the normal
	plane.m_normal = Vec4(trf.getRotation() * m_normal, 0.0);

	// the offset
	Mat3x4 rot = trf.getRotation();
	rot.transposeRotationPart();
	Vec4 newTrans(rot * trf.getOrigin(), 0.0);
	plane.m_offset = m_offset * trf.getScale() + newTrans.dot(m_normal);

	return plane;
}

//==============================================================================
void Plane::computeAabb(Aabb&) const
{
	ANKI_ASSERT(0 && "Can't do that");
}

} // end namespace anki
