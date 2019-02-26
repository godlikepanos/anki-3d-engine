// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
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
inline TVec4<F32> TMat4F32Base::operator*(const TVec4<F32>& b) const
{
	TVec4<F32> v;

	for(U i = 0; i < 4; i++)
	{
		_mm_store_ss(&v[i], _mm_dp_ps(m_simd[i], b.getSimd(), 0xF1));
	}

	return v;
}

#elif ANKI_SIMD == ANKI_SIMD_NEON

#	error "TODO"

#endif

} // end namespace anki
