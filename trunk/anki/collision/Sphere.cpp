#include "anki/collision/Sphere.h"
#include "anki/collision/Plane.h"


//==============================================================================
// getTransformed                                                              =
//==============================================================================
Sphere Sphere::getTransformed(const Transform& transform) const
{
	Sphere newSphere;
	newSphere.center = center.getTransformed(transform.getOrigin(),
		transform.getRotation(), transform.getScale());
	newSphere.radius = radius * transform.getScale();
	return newSphere;
}


//==============================================================================
// getCompoundShape                                                            =
//==============================================================================
Sphere Sphere::getCompoundShape(const Sphere& b) const
{
	const Sphere& a = *this;

	/// @todo test this
	/*
	Vec3 centerDiff = b.center - a.center;
	float radiusDiff = b.radius - a.radius;
	Vec3 radiusDiffSqr = radiusDiff * radiusDiff;
	float lenSqr = centerDiff.getLengthSquared();

	if(radiusDiffSqrt >= 0.0)
	{
		if(radiusDiff >= 0.0)
		{
			return b;
		}
		else
		{
			return a;
		}
	}
	else
	{
		float l = sqrt(lenSqr);
		float t = (l + b.radius - a.radius) / (2.0 * l);
		return Sphere(a.center + t * centerDiff, (l + a.radius + b.radius) /
			2.0);
	}
	*/

	Vec3 c = b.getCenter() - a.getCenter(); // Vector from one center to the
	                                        // other
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


//==============================================================================
// testPlane                                                                   =
//==============================================================================
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

