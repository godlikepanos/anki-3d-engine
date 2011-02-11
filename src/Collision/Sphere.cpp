#include "Sphere.h"
#include "Plane.h"


//======================================================================================================================
// getTransformed                                                                                                      =
//======================================================================================================================
Sphere Sphere::getTransformed(const Transform& transform) const
{
	Sphere newSphere;
	newSphere.center = center.getTransformed(transform.getOrigin(), transform.getRotation(), transform.getScale());
	newSphere.radius = radius * transform.getScale();
	return newSphere;
}


//======================================================================================================================
// getCompoundSphere                                                                                                   =
//======================================================================================================================
Sphere Sphere::getCompoundSphere(const Sphere& b) const
{
	const Sphere& a = *this;

	Vec3 c = b.getCenter() - a.getCenter(); // vector from one center to the other
	float cLen = c.getLength();

	if(cLen + b.getRadius() < a.getRadius())
	{
		return a;
	}
	else if(cLen + a.getRadius() < b.getRadius())
	{
		return b;
	}

	Vec3 bnorm = c / cLen;

	Vec3 ca = (-bnorm) * a.getRadius() + a.getCenter();
	Vec3 cb = (bnorm) * b.getRadius() + b.getCenter();

	return Sphere((ca + cb) / 2.0, (ca - cb).getLength() / 2.0);
}


//======================================================================================================================
// testPlane                                                                                                           =
//======================================================================================================================
float Sphere::testPlane(const Plane& plane) const
{
	float dist = plane.test(center);

	if(dist > radius)
	{
		return dist - radius;
	}
	else if(-dist > radius)
	{
		return dist + radius;
	}
	else
	{
		return 0.0;
	}
}
