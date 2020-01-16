// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/collision/Sphere.h>

namespace anki
{

Sphere Sphere::getCompoundShape(const Sphere& b) const
{
	const Sphere& a = *this;

	const Vec4 c = b.getCenter() - a.getCenter();
	const F32 cLen = c.getLength();

	if(cLen + b.getRadius() < a.getRadius())
	{
		return a;
	}
	else if(cLen + a.getRadius() < b.getRadius())
	{
		return b;
	}

	const Vec4 bnorm = c / cLen;

	const Vec4 ca = (-bnorm) * a.getRadius() + a.getCenter();
	const Vec4 cb = (bnorm)*b.getRadius() + b.getCenter();

	return Sphere((ca + cb) / 2.0f, (ca - cb).getLength() / 2.0f);
}

void Sphere::setFromPointCloud(const Vec3* pointBuffer, U pointCount, PtrSize pointStride, PtrSize buffSize)
{
	// Calc center
	{
		Vec4 min(Vec3(MAX_F32), 0.0f);
		Vec4 max(Vec3(MIN_F32), 0.0f);

		// Iterate
		const U8* ptr = reinterpret_cast<const U8*>(pointBuffer);
		while(pointCount-- != 0)
		{
			ANKI_ASSERT((ptrToNumber(ptr) + sizeof(Vec3) - ptrToNumber(pointBuffer)) <= buffSize);
			const Vec4 pos = (*reinterpret_cast<const Vec3*>(ptr)).xyz0();

			max = max.max(pos);
			min = min.min(pos);

			ptr += pointStride;
		}

		m_center = (min + max) * 0.5f; // average
	}

	// Calc radius
	{
		F32 maxDist = MIN_F32;

		const U8* ptr = reinterpret_cast<const U8*>(pointBuffer);
		while(pointCount-- != 0)
		{
			ANKI_ASSERT((ptrToNumber(ptr) + sizeof(Vec3) - ptrToNumber(pointBuffer)) <= buffSize);
			const Vec3& pos = *reinterpret_cast<const Vec3*>(ptr);

			const F32 dist = (Vec4(pos, 0.0f) - m_center).getLengthSquared();
			if(dist > maxDist)
			{
				maxDist = dist;
			}

			ptr += pointStride;
		}

		m_radius = sqrt(maxDist);
	}
}

} // end namespace anki
