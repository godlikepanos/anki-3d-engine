#include "anki/collision/Aabb.h"
#include "anki/collision/Plane.h"
#include <boost/array.hpp>


namespace anki {


//==============================================================================
float Aabb::testPlane(const Plane& plane) const
{
	Vec3 diagMin, diagMax;
	// set min/max values for x,y,z direction
	for(int i = 0; i < 3; i++)
	{
		if(plane.getNormal()[i] >= 0.0)
		{
			diagMin[i] = min[i];
			diagMax[i] = max[i];
		}
		else
		{
			diagMin[i] = max[i];
			diagMax[i] = min[i];
		}
	}

	// minimum on positive side of plane, box on positive side
	float test = plane.test(diagMin);
	if(test > 0.0)
	{
		return test;
	}

	test = plane.test(diagMax);
	// min on non-positive side, max on non-negative side, intersection
	if(test >= 0.0)
	{
		return 0.0;
	}
	// max on negative side, box on negative side
	else
	{
		return test;
	}
}


//==============================================================================
Aabb Aabb::getTransformed(const Transform& transform) const
{
	Aabb out;

	// if there is no rotation our job is easy
	if(transform.getRotation() == Mat3::getIdentity())
	{
		out.min = (min * transform.getScale()) + transform.getOrigin();
		out.max = (max * transform.getScale()) + transform.getOrigin();
	}
	// if not then we are fucked
	else
	{
		boost::array<Vec3, 8> points = {{
			max,
			Vec3(min.x(), max.y(), max.z()),
			Vec3(min.x(), min.y(), max.z()),
			Vec3(max.x(), min.y(), max.z()),
			Vec3(max.x(), max.y(), min.z()),
			Vec3(min.x(), max.y(), min.z()),
			min,
			Vec3(max.x(), min.y(), min.z())
		}};

		for(size_t i = 0; i < points.size(); i++)
		{
			points[i].transform(transform);
		}

		out.set(points);
	}

	return out;
}


//==============================================================================
Aabb Aabb::getCompoundShape(const Aabb& b) const
{
	Aabb out;

	for(uint i = 0; i < 3; i++)
	{
		out.min[i] = (min[i] < b.min[i]) ? min[i] : b.min[i];
		out.max[i] = (max[i] > b.max[i]) ? max[i] : b.max[i];
	}

	return out;
}


} // namespace anki
