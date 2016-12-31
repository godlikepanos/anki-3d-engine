// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/collision/LineSegment.h>
#include <anki/collision/Plane.h>
#include <anki/collision/Aabb.h>
#include <algorithm>

namespace anki
{

F32 LineSegment::testPlane(const Plane& p) const
{
	const Vec4& p0 = m_origin;
	Vec4 p1 = m_origin + m_dir;

	F32 dist0 = p.test(p0);
	F32 dist1 = p.test(p1);

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

LineSegment LineSegment::getTransformed(const Transform& trf) const
{
	LineSegment out;
	out.m_origin = trf.transform(m_origin);
	out.m_dir = Vec4(trf.getRotation() * (m_dir * trf.getScale()), 0.0);
	return out;
}

void LineSegment::computeAabb(Aabb& aabb) const
{
	Vec4 min = m_origin;
	Vec4 max = m_origin + m_dir;

	for(U i = 0; i < 3; ++i)
	{
		if(max[i] < min[i])
		{
			F32 tmp = max[i];
			max[i] = min[i];
			min[i] = tmp;
		}
	}

	aabb.setMin(min);
	aabb.setMax(max);
}

} // end namespace
