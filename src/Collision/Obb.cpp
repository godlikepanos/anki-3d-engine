#include "Obb.h"
#include "Plane.h"


//======================================================================================================================
// getTransformed                                                                                                      =
//======================================================================================================================
Obb Obb::getTransformed(const Transform& transform) const
{
	Obb out;
	out.extends = extends * transform.getScale();
	out.center = center.getTransformed(transform);
	out.rotation = transform.getRotation() * rotation;
	return out;
}


//======================================================================================================================
// testPlane                                                                                                           =
//======================================================================================================================
float Obb::testPlane(const Plane& plane) const
{
	Vec3 xNormal = rotation.getTransposed() * plane.getNormal();

	// maximum extent in direction of plane normal
	float r = fabs(extends.x() * xNormal.x()) + fabs(extends.y() * xNormal.y()) + fabs(extends.z() * xNormal.z());
	// signed distance between box center and plane
	float d = plane.test(center);

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
