// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/OccluderComponent.h>

namespace anki
{

void OccluderComponent::setVertices(const Vec3* begin, U32 count, U32 stride)
{
	ANKI_ASSERT(begin);
	ANKI_ASSERT(count > 0 && (count % 3) == 0);
	ANKI_ASSERT(stride >= sizeof(Vec3));

	m_begin = begin;
	m_count = count;
	m_stride = stride;

	Vec3 minv(MAX_F32), maxv(MIN_F32);
	while(count--)
	{
		const Vec3& v = *reinterpret_cast<const Vec3*>(reinterpret_cast<const U8*>(begin) + stride * count);
		for(U i = 0; i < 3; ++i)
		{
			minv[i] = min(minv[i], v[i]);
			maxv[i] = max(maxv[i], v[i]);
		}
	}

	m_aabb.setMin(minv.xyz0());
	m_aabb.setMax(maxv.xyz0());
}

} // end namespace anki
