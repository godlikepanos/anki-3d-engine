// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/collision/ConvexHullShape.h>
#include <anki/util/Functions.h>

namespace anki
{

ConvexHullShape ConvexHullShape::getTransformed(const Transform& trf) const
{
	check();

	ConvexHullShape out = *this;

	out.m_trf = m_trf.combineTransformations(trf);
	out.m_invTrf = m_trf.getInverse();
	out.m_trfIdentity = false;

	return out;
}

void ConvexHullShape::setTransform(const Transform& trf)
{
	m_trf = trf;
	m_invTrf = m_trf.getInverse();
	m_trfIdentity = false;
}

Vec4 ConvexHullShape::computeSupport(const Vec4& dir) const
{
	check();

	F32 m = MIN_F32;
	U index = 0;

	const Vec4 d = (m_trfIdentity) ? dir : (m_invTrf.getRotation() * dir).xyz0();

	const Vec4* points = m_points;
	const Vec4* end = m_points + m_pointCount;
	U i = 0;
	for(; points != end; ++points, ++i)
	{
		const F32 dot = points->dot(d);
		if(dot > m)
		{
			m = dot;
			index = i;
		}
	}

	return (m_trfIdentity) ? m_points[index] : m_trf.transform(m_points[index]);
}

} // end namespace anki
