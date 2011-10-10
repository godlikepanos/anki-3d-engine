#include "anki/collision/Ray.h"
#include "anki/collision/Plane.h"


//==============================================================================
// getTransformed                                                              =
//==============================================================================
Ray Ray::getTransformed(const Transform& transform) const
{
	Ray out;
	out.origin = origin.getTransformed(transform);
	out.dir = transform.getRotation() * dir;
	return out;
}


//==============================================================================
// testPlane                                                                   =
//==============================================================================
float Ray::testPlane(const Plane& plane) const
{
	float dist = plane.test(origin);
	float cos_ = plane.getNormal().dot(dir);

	if(cos_ > 0.0) // the ray points to the same half-space as the plane
	{
		if(dist < 0.0) // the ray's origin is behind the plane
		{
			return 0.0;
		}
		else
		{
			return dist;
		}
	}
	else
	{
		if(dist > 0.0)
		{
			return 0.0;
		}
		else
		{
			return dist;
		}
	}
}
