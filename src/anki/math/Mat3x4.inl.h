// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/math/CommonSrc.h>

namespace anki
{

#if ANKI_SIMD == ANKI_SIMD_SSE

template<>
inline TMat3x4<F32>::Base::TMat(const TMat3x4<F32>::Base& b)
{
	for(U i = 0; i < 3; i++)
	{
		m_simd[i] = b.m_simd[i];
	}
}

template<>
inline TMat3x4<F32>::Base::TMat(const F32 f)
{
	for(U i = 0; i < 3; i++)
	{
		m_simd[i] = _mm_set1_ps(f);
	}
}

template<>
inline TVec3<F32> TMat3x4<F32>::Base::operator*(const TVec4<F32>& b) const
{
	TVec3<F32> v;

	for(U i = 0; i < 3; i++)
	{
		_mm_store_ss(&v[i], _mm_dp_ps(m_simd[i], b.getSimd(), 0xF1));
	}

	return v;
}

template<>
inline TMat3x4<F32> TMat3x4<F32>::combineTransformations(const TMat3x4<F32>& b) const
{
	TMat3x4<F32> c;
	const TMat3x4<F32>& a = *static_cast<const TMat3x4<F32>*>(this);

	for(U i = 0; i < 3; i++)
	{
		__m128 t1, t2;

		t1 = _mm_set1_ps(a(i, 0));
		t2 = _mm_mul_ps(b.m_simd[0], t1);
		t1 = _mm_set1_ps(a(i, 1));
		t2 = _mm_add_ps(_mm_mul_ps(b.m_simd[1], t1), t2);
		t1 = _mm_set1_ps(a(i, 2));
		t2 = _mm_add_ps(_mm_mul_ps(b.m_simd[2], t1), t2);

		TVec4<F32> v4(0.0, 0.0, 0.0, a(i, 3));
		t2 = _mm_add_ps(v4.getSimd(), t2);

		c.m_simd[i] = t2;
	}

	return c;
}

#endif

} // end namespace anki
