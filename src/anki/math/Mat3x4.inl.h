// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/math/CommonSrc.h>

namespace anki
{

#if ANKI_SIMD == ANKI_SIMD_SSE

template<>
inline TVec3<F32> TMat3x4F32Base::operator*(const TVec4<F32>& b) const
{
	TVec3<F32> v;

	for(U i = 0; i < 3; i++)
	{
		_mm_store_ss(&v[i], _mm_dp_ps(m_simd[i], b.getSimd(), 0xF1));
	}

	return v;
}

#endif

} // end namespace anki
