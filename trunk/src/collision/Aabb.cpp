#include "anki/collision/Aabb.h"
#include "anki/collision/Plane.h"
#include "anki/collision/Obb.h"
#include <array>

namespace anki {

//==============================================================================
Aabb Aabb::getTransformed(const Transform& transform) const
{
	Mat3 absM;
	for(U i = 0; i < 9; ++i)
	{
		absM[i] = fabs(transform.getRotation()[i]);
	}

	Vec3 center = (min + max) * 0.5;
	Vec3 extend = (max - min) * 0.5;

	Vec3 newC = center.getTransformed(transform);
	Vec3 newE = absM * (extend * transform.getScale());

	return Aabb(newC - newE, newC + newE);
}

//==============================================================================
F32 Aabb::testPlane(const Plane& p) const
{
	const Aabb& aabb = *this;
	Vec3 diagMin, diagMax;
	// set min/max values for x,y,z direction
	for(U i = 0; i < 3; i++)
	{
		if(p.getNormal()[i] >= 0.0)
		{
			diagMin[i] = aabb.getMin()[i];
			diagMax[i] = aabb.getMax()[i];
		}
		else
		{
			diagMin[i] = aabb.getMax()[i];
			diagMax[i] = aabb.getMin()[i];
		}
	}

	// minimum on positive side of plane, box on positive side
	F32 test = p.test(diagMin);
	if(test > 0.0)
	{
		return test;
	}

	test = p.test(diagMax);
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
Aabb Aabb::getCompoundShape(const Aabb& b) const
{
	Aabb out;

	for(U i = 0; i < 3; i++)
	{
		out.min[i] = (min[i] < b.min[i]) ? min[i] : b.min[i];
		out.max[i] = (max[i] > b.max[i]) ? max[i] : b.max[i];
	}

	return out;
}

} // namespace anki
