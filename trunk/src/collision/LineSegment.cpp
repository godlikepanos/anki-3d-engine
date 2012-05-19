#include "anki/collision/LineSegment.h"
#include "anki/collision/Plane.h"
#include "anki/collision/Aabb.h"
#include <algorithm>


namespace anki {


//==============================================================================
float LineSegment::testPlane(const Plane& p) const
{
	const LineSegment& ls = *this;
	const Vec3& p0 = ls.getOrigin();
	Vec3 p1 = ls.getOrigin() + ls.getDirection();

	float dist0 = p.test(p0);
	float dist1 = p.test(p1);

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


//==============================================================================
LineSegment LineSegment::getTransformed(const Transform& transform) const
{
	LineSegment out;
	out.origin = origin.getTransformed(transform);
	out.dir = transform.getRotation() * (dir * transform.getScale());
	return out;
}


//==============================================================================
void LineSegment::getAabb(Aabb& aabb) const
{
	Vec3 min = origin;
	Vec3 max = origin + dir;

	for(uint i = 0; i < 3; ++i)
	{
		if(max[i] < min[i])
		{
			float tmp = max[i];
			max[i] = min[i];
			min[i] = tmp;
		}
	}

	aabb.setMin(min);
	aabb.setMax(max);
}


} // end namespace
