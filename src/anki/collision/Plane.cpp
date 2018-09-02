// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/collision/Plane.h>
#include <anki/util/Assert.h>

namespace anki
{

F32 Plane::testPlane(const Plane& /*p*/) const
{
	ANKI_ASSERT(0 && "Ambiguous call");
	return 0.0;
}

void Plane::setFrom3Points(const Vec4& p0, const Vec4& p1, const Vec4& p2)
{
	ANKI_ASSERT(p0.w() == 0.0 && p1.w() == 0.0 && p2.w() == 0.0);

	// get plane vectors
	Vec4 u = p1 - p0;
	Vec4 v = p2 - p0;

	m_normal = u.cross(v);

	// length of normal had better not be zero
	ANKI_ASSERT(!isZero(m_normal.getLengthSquared()));

	m_normal.normalize();
	m_offset = m_normal.dot(p0);
}

void Plane::setFromPlaneEquation(F32 a, F32 b, F32 c, F32 d)
{
	m_normal = Vec4(a, b, c, 0.0);

	// length of normal had better not be zero
	ANKI_ASSERT(isZero(m_normal.getLength() - 1.0));

	m_offset = d;
}

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

void Plane::computeAabb(Aabb&) const
{
	ANKI_ASSERT(0 && "Can't do that");
}

Bool Plane::intersectVector(const Vec4& p, Vec4& intersection) const
{
	ANKI_ASSERT(p.w() == 0.0);
	Vec4 pp = p.getNormalized();
	F32 dot = pp.dot(m_normal);

	if(!isZero(dot))
	{
		F32 s = m_offset / dot;
		intersection = pp * s;
		return true;
	}
	else
	{
		return false;
	}
}

Bool Plane::intersectRay(const Vec4& rayOrigin, const Vec4& rayDir, Vec4& intersection) const
{
	ANKI_ASSERT(rayOrigin.w() == 0.0 && rayDir.w() == 0.0);
	Bool intersects = false;

	F32 d = test(rayOrigin); // Dist of origin to the plane
	F32 a = m_normal.dot(rayDir);

	if(d > 0.0 && a < 0.0)
	{
		// To have intersection the d should be positive and the s as well. So the 'a' must be negative and not zero
		// because of the division.
		F32 s = -d / a;
		ANKI_ASSERT(s > 0.0);

		intersection = rayOrigin + s * rayDir;
		intersects = true;
	}

	return intersects;
}

void Plane::extractClipPlane(const Mat4& mvp, FrustumPlaneType id, Plane& plane)
{
#define ANKI_CASE(i, a, op, b) \
	case i: \
	{ \
		const Vec4 planeEqationCoefs = mvp.getRow(a) op mvp.getRow(b); \
		const Vec4 n = planeEqationCoefs.xyz0(); \
		const F32 len = n.getLength(); \
		plane = Plane(n / len, -planeEqationCoefs.w() / len); \
		break; \
	}

	switch(id)
	{
		ANKI_CASE(FrustumPlaneType::NEAR, 3, +, 2)
		ANKI_CASE(FrustumPlaneType::FAR, 3, -, 2)
		ANKI_CASE(FrustumPlaneType::LEFT, 3, +, 0)
		ANKI_CASE(FrustumPlaneType::RIGHT, 3, -, 0)
		ANKI_CASE(FrustumPlaneType::TOP, 3, -, 1)
		ANKI_CASE(FrustumPlaneType::BOTTOM, 3, +, 1)
	default:
		ANKI_ASSERT(0);
	}

#undef ANKI_CASE
}

void Plane::extractClipPlanes(const Mat4& mvp, Array<Plane, 6>& planes)
{
	for(U i = 0; i < 6; ++i)
	{
		extractClipPlane(mvp, static_cast<FrustumPlaneType>(i), planes[i]);
	}
}

} // end namespace anki
