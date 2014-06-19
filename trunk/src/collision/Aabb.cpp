// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/collision/Aabb.h"
#include "anki/collision/Plane.h"
#include "anki/collision/Obb.h"
#include <array>

namespace anki {

//==============================================================================
Aabb Aabb::getTransformed(const Transform& trf) const
{
	Mat3x4 absM;
	for(U i = 0; i < 12; ++i)
	{
		absM[i] = fabs(trf.getRotation()[i]);
	}

	Vec4 center = (m_min + m_max) * 0.5;
	Vec4 extend = (m_max - m_min) * 0.5;

	Vec4 newC = trf.transform(center);
	Vec4 newE = Vec4(absM * (extend * trf.getScale()), 0.0);

	return Aabb(newC - newE, newC + newE);
}

//==============================================================================
F32 Aabb::testPlane(const Plane& p) const
{
	const Aabb& aabb = *this;
	Vec4 diagMin(0.0), diagMax(0.0);
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
	out.m_min.w() = out.m_max.w() = 0.0;

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
	m_min = Vec4(Vec3(MAX_F32), 0.0);
	m_max = Vec4(Vec3(MIN_F32), 0.0);

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

//==============================================================================
Vec4 Aabb::computeSupport(const Vec4& dir) const
{
	Vec4 ret(0.0);
 
	ret.x() = dir.x() >= 0.0 ? m_max.x() : m_min.x();
	ret.y() = dir.y() >= 0.0 ? m_max.y() : m_min.y();
	ret.z() = dir.z() >= 0.0 ? m_max.z() : m_min.z();
 
	return ret;
}

} // end namespace anki
