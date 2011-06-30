#include "LineSegment.h"
#include "Plane.h"
#include <algorithm>


namespace Col {


//==============================================================================
// getTransformed                                                              =
//==============================================================================
LineSegment LineSegment::getTransformed(const Transform& transform) const
{
	LineSegment out;
	out.origin = origin.getTransformed(transform);
	out.dir = transform.getRotation() * (dir * transform.getScale());
	return out;
}


//==============================================================================
// testPlane                                                                   =
//==============================================================================
float LineSegment::testPlane(const Plane& plane) const
{
	const Vec3& p0 = origin;
	Vec3 p1 = origin + dir;

	float dist0 = plane.test(p0);
	float dist1 = plane.test(p1);

	if(dist0 > 0.0)
	{
		if(dist1 > 0.0)
		{
			return std::min(dist0, dist1);
		}
		else
		{
			return 0.0;
		}
	}
	else
	{
		if(dist1 < 0.0)
		{
			return std::max(dist0, dist1);
		}
		else
		{
			return 0.0;
		}
	}
}


} // end namespace
