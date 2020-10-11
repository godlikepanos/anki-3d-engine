// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/collision/Aabb.h>

namespace anki
{

Aabb Aabb::getTransformed(const Transform& trf) const
{
	Mat3x4 absM;
	for(U i = 0; i < 12; ++i)
	{
		absM[i] = absolute(trf.getRotation()[i]);
	}

	Vec4 center = (m_min + m_max) * 0.5;
	Vec4 extend = (m_max - m_min) * 0.5;

	Vec4 newC = trf.transform(center);
	Vec4 newE = Vec4(absM * (extend * trf.getScale()), 0.0);

	return Aabb(newC - newE, newC + newE);
}

Aabb Aabb::getCompoundShape(const Aabb& b) const
{
	Aabb out;
	out.m_min.w() = out.m_max.w() = 0.0f;

	for(U i = 0; i < 3; i++)
	{
		out.m_min[i] = (m_min[i] < b.m_min[i]) ? m_min[i] : b.m_min[i];
		out.m_max[i] = (m_max[i] > b.m_max[i]) ? m_max[i] : b.m_max[i];
	}

	return out;
}

void Aabb::setFromPointCloud(const Vec3* pointBuffer, U pointCount, PtrSize pointStride, PtrSize buffSize)
{
	// Preconditions
	ANKI_ASSERT(pointBuffer);
	ANKI_ASSERT(pointCount > 1);
	ANKI_ASSERT(pointStride >= sizeof(Vec3));
	ANKI_ASSERT(buffSize >= pointStride * pointCount);

	m_min = Vec4(Vec3(MAX_F32), 0.0f);
	m_max = Vec4(Vec3(MIN_F32), 0.0f);

	// Iterate
	const U8* ptr = reinterpret_cast<const U8*>(pointBuffer);
	while(pointCount-- != 0)
	{
		ANKI_ASSERT((ptrToNumber(ptr) + sizeof(Vec3) - ptrToNumber(pointBuffer)) <= buffSize);
		const Vec3& pos = *reinterpret_cast<const Vec3*>(ptr);

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

		ptr += pointStride;
	}
}

Vec4 Aabb::computeSupport(const Vec4& dir) const
{
	Vec4 ret(0.0f);

	ret.x() = (dir.x() >= 0.0f) ? m_max.x() : m_min.x();
	ret.y() = (dir.y() >= 0.0f) ? m_max.y() : m_min.y();
	ret.z() = (dir.z() >= 0.0f) ? m_max.z() : m_min.z();

	return ret;
}

} // end namespace anki
