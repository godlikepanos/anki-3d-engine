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

	Vec3 center = (m_min + m_max) * 0.5;
	Vec3 extend = (m_max - m_min) * 0.5;

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
		out.m_min[i] = (m_min[i] < b.m_min[i]) ? m_min[i] : b.m_min[i];
		out.m_max[i] = (m_max[i] > b.m_max[i]) ? m_max[i] : b.m_max[i];
	}

	return out;
}

//==============================================================================
void Aabb::setFromPointCloud(
	const void* buff, U count, PtrSize stride, PtrSize buffSize)
{
	m_min = Vec3(MAX_F32);
	m_max = Vec3(MIN_F32);

	iteratePointCloud(buff, count, stride, buffSize, [&](const Vec3& pos)
	{
		for(U j = 0; j < 3; j++)
		{
			if(pos[j] > m_max[j])
			{
				m_max[j] = pos[j];
			}
			else if(pos[j] < m_min[j])
			{
				m_min[j] = pos[j];
			}
		}
	});
}

} // end namespace anki
