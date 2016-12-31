// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/math/CommonSrc.h>

namespace anki
{

/// @memberof TMat4
template<typename T>
TMat4<T> operator+(const T f, const TMat4<T>& m4)
{
	return m4 + f;
}

/// @memberof TMat4
template<typename T>
TMat4<T> operator-(const T f, const TMat4<T>& m4)
{
	TMat4<T> out;
	for(U i = 0; i < 16; i++)
	{
		out[i] = f - m4[i];
	}
	return out;
}

/// @memberof TMat4
template<typename T>
TMat4<T> operator*(const T f, const TMat4<T>& m4)
{
	return m4 * f;
}

/// @memberof TMat4
template<typename T>
TMat4<T> operator/(const T f, const TMat4<T>& m4)
{
	TMat4<T> out;
	for(U i = 0; i < 16; i++)
	{
		out[i] = f / m4[i];
	}
	return out;
}

#if ANKI_SIMD == ANKI_SIMD_SSE

template<>
inline TMat4<F32>::Base::TMat(const TMat4<F32>::Base& b)
{
	for(U i = 0; i < 4; i++)
	{
		m_simd[i] = b.m_simd[i];
	}
}

template<>
inline TMat4<F32>::Base::TMat(const F32 f)
{
	for(U i = 0; i < 4; i++)
	{
		m_simd[i] = _mm_set1_ps(f);
	}
}

template<>
inline TMat4<F32>& TMat4<F32>::Base::operator=(const TMat4<F32>& b)
{
	for(U i = 0; i < 4; i++)
	{
		m_simd[i] = b.m_simd[i];
	}
	return static_cast<TMat4<F32>&>(*this);
}

template<>
inline TMat4<F32> TMat4<F32>::Base::operator+(const TMat4<F32>& b) const
{
	TMat4<F32> c;
	for(U i = 0; i < 4; i++)
	{
		c.m_simd[i] = _mm_add_ps(m_simd[i], b.m_simd[i]);
	}
	return c;
}

template<>
inline TMat4<F32>& TMat4<F32>::Base::operator+=(const TMat4<F32>& b)
{
	for(U i = 0; i < 4; i++)
	{
		m_simd[i] = _mm_add_ps(m_simd[i], b.m_simd[i]);
	}
	return static_cast<TMat4<F32>&>(*this);
}

template<>
inline TMat4<F32> TMat4<F32>::Base::operator-(const TMat4<F32>& b) const
{
	TMat4<F32> c;
	for(U i = 0; i < 4; i++)
	{
		c.m_simd[i] = _mm_sub_ps(m_simd[i], b.m_simd[i]);
	}
	return c;
}

template<>
inline TMat4<F32>& TMat4<F32>::Base::operator-=(const TMat4<F32>& b)
{
	for(U i = 0; i < 4; i++)
	{
		m_simd[i] = _mm_sub_ps(m_simd[i], b.m_simd[i]);
	}
	return static_cast<TMat4<F32>&>(*this);
}

template<>
inline TMat4<F32> TMat4<F32>::Base::operator*(const TMat4<F32>& b) const
{
	TMat4<F32> out;
	const TMat4<F32>& m = *static_cast<const TMat4<F32>*>(this);

	for(U i = 0; i < 4; i++)
	{
		__m128 t1, t2;

		t1 = _mm_set1_ps(m(i, 0));
		t2 = _mm_mul_ps(b.m_simd[0], t1);
		t1 = _mm_set1_ps(m(i, 1));
		t2 = _mm_add_ps(_mm_mul_ps(b.m_simd[1], t1), t2);
		t1 = _mm_set1_ps(m(i, 2));
		t2 = _mm_add_ps(_mm_mul_ps(b.m_simd[2], t1), t2);
		t1 = _mm_set1_ps(m(i, 3));
		t2 = _mm_add_ps(_mm_mul_ps(b.m_simd[3], t1), t2);

		out.m_simd[i] = t2;
	}

	return out;
}

template<>
inline TVec4<F32> TMat4<F32>::Base::operator*(const TVec4<F32>& b) const
{
	TVec4<F32> v;

	for(U i = 0; i < 4; i++)
	{
		_mm_store_ss(&v[i], _mm_dp_ps(m_simd[i], b.getSimd(), 0xF1));
	}

	return v;
}

template<>
inline void TMat4<F32>::Base::setRows(
	const TVec4<F32>& a, const TVec4<F32>& b, const TVec4<F32>& c, const TVec4<F32>& d)
{
	m_simd[0] = a.getSimd();
	m_simd[1] = b.getSimd();
	m_simd[2] = c.getSimd();
	m_simd[3] = d.getSimd();
}

template<>
inline void TMat4<F32>::Base::setRow(const U i, const TVec4<F32>& v)
{
	m_simd[i] = v.getSimd();
}

template<>
inline void TMat4<F32>::Base::transpose()
{
	_MM_TRANSPOSE4_PS(m_simd[0], m_simd[1], m_simd[2], m_simd[3]);
}

#elif ANKI_SIMD == ANKI_SIMD_NEON

#error "TODO"

#endif

} // end namespace anki
