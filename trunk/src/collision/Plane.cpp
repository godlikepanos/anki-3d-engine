#include "anki/collision/Plane.h"
#include "anki/util/Assert.h"

namespace anki {

//==============================================================================
Plane::Plane(const Plane& b)
	: CollisionShape(CST_PLANE), normal(b.normal), offset(b.offset)
{}

//==============================================================================
Plane::Plane(const Vec3& normal_, F32 offset_)
	: CollisionShape(CST_PLANE), normal(normal_), offset(offset_)
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
	Vec3 u = p1 - p0;
	Vec3 v = p2 - p0;

	normal = u.cross(v);

	// length of normal had better not be zero
	ANKI_ASSERT(!isZero(normal.getLengthSquared()));

	normal.normalize();
	offset = normal.dot(p0); // XXX: correct??
}

//==============================================================================
void Plane::setFromPlaneEquation(F32 a, F32 b, F32 c, F32 d)
{
	// normalize for cheap distance checks
	F32 lensq = a * a + b * b + c * c;
	// length of normal had better not be zero
	ANKI_ASSERT(!isZero(lensq));

	// recover gracefully
	if(isZero(lensq))
	{
		normal = Vec3(1.0, 0.0, 0.0);
		offset = 0.0;
	}
	else
	{
		F32 recip = 1.0 / sqrt(lensq);
		normal = Vec3(a * recip, b * recip, c * recip);
		offset = d * recip;
	}
}

//==============================================================================
Plane Plane::getTransformed(const Transform& trf) const
{
	Plane plane;

	// the normal
	plane.normal = trf.getRotation() * normal;

	// the offset
	Vec3 newTrans = trf.getRotation().getTransposed() * trf.getOrigin();
	plane.offset = offset * trf.getScale() + newTrans.dot(normal);

	return plane;
}

//==============================================================================
void Plane::toAabb(Aabb&) const
{
	ANKI_ASSERT(0 && "Can't do that");
}

} // end namespace
