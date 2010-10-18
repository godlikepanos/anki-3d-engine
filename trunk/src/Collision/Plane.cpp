#include "Plane.h"
#include "Exception.h"


//======================================================================================================================
// setFrom3Points                                                                                                      =
//======================================================================================================================
void Plane::setFrom3Points(const Vec3& p0, const Vec3& p1, const Vec3& p2)
{
	// get plane vectors
	Vec3 u = p1 - p0;
	Vec3 v = p2 - p0;

	normal = u.cross(v);

	// length of normal had better not be zero
	RASSERT_THROW_EXCEPTION(isZero(normal.getLengthSquared()));

	normal.normalize();
	offset = normal.dot(p0); // ToDo: correct??
}


//======================================================================================================================
// setFromPlaneEquation                                                                                                =
//======================================================================================================================
void Plane::setFromPlaneEquation(float a, float b, float c, float d)
{
	// normalize for cheap distance checks
	float lensq = a * a + b * b + c * c;
	// length of normal had better not be zero
	RASSERT_THROW_EXCEPTION(isZero(lensq));

	// recover gracefully
	if(isZero(lensq))
	{
		normal = Vec3(1.0, 0.0, 0.0);
		offset = 0.0;
	}
	else
	{
		float recip = invSqrt(lensq);
		normal = Vec3(a * recip, b * recip, c * recip);
		offset = d * recip;
	}
}


//======================================================================================================================
// getTransformed                                                                                                      =
//======================================================================================================================
Plane Plane::getTransformed(const Vec3& translate, const Mat3& rotate, float scale) const
{
	Plane plane;

	// the normal
	plane.normal = rotate * normal;

	// the offset
	Vec3 new_trans = rotate.getTransposed() * translate;
	plane.offset = offset*scale + new_trans.dot(normal);

	return plane;
}
