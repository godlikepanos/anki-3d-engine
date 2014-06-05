// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/math/CommonSrc.h"

namespace anki {

//==============================================================================
// Friends                                                                     =
//==============================================================================

//==============================================================================
/// @memberof TVec4
template<typename T>
TVec4<T> operator+(const T f, const TVec4<T>& v4)
{
	return v4 + f;
}

//==============================================================================
/// @memberof TVec4
template<typename T>
TVec4<T> operator-(const T f, const TVec4<T>& v4)
{
	return TVec4<T>(f) - v4;
}

//==============================================================================
/// @memberof TVec4
template<typename T>
TVec4<T> operator*(const T f, const TVec4<T>& v4)
{
	return v4 * f;
}

//==============================================================================
/// @memberof TVec4
template<typename T>
TVec4<T> operator/(const T f, const TVec4<T>& v4)
{
	return TVec4<T>(f) / v4;
}

#if ANKI_SIMD == ANKI_SIMD_SSE

//==============================================================================
// SSE specializations                                                         =
//==============================================================================

//==============================================================================
// Constructors                                                                =
//==============================================================================

//==============================================================================
template<>
inline TVec4<F32>::TVec4(F32 f)
{
	m_simd = _mm_set1_ps(f);
}

//==============================================================================
template<>
inline TVec4<F32>::TVec4(const F32 arr[])
{
	m_simd = _mm_load_ps(arr);
}

//==============================================================================
template<>
inline TVec4<F32>::TVec4(const F32 x_, const F32 y_, const F32 z_, 
	const F32 w_)
{
	m_simd = _mm_set_ps(w_, z_, y_, x_);
}

//==============================================================================
template<>
inline TVec4<F32>::TVec4(const TVec4<F32>& b)
	: Base()
{
	m_simd = b.m_simd;
}

//==============================================================================
// Operators with same                                                         =
//==============================================================================

//==============================================================================
template<>
inline TVec4<F32>& TVec4<F32>::Base::operator=(const TVec4<F32>& b)
{
	m_simd = b.m_simd;
	return static_cast<TVec4<F32>&>(*this);
}

//==============================================================================
template<>
inline TVec4<F32> TVec4<F32>::Base::operator+(const TVec4<F32>& b) const
{
	return TVec4<F32>(_mm_add_ps(m_simd, b.m_simd));
}

//==============================================================================
template<>
inline TVec4<F32>& TVec4<F32>::Base::operator+=(const TVec4<F32>& b)
{
	m_simd = _mm_add_ps(m_simd, b.m_simd);
	return static_cast<TVec4<F32>&>(*this);
}

//==============================================================================
template<>
inline TVec4<F32> TVec4<F32>::Base::operator-(const TVec4<F32>& b) const
{
	return TVec4<F32>(_mm_sub_ps(m_simd, b.m_simd));
}

//==============================================================================
template<>
inline TVec4<F32>& TVec4<F32>::Base::operator-=(const TVec4<F32>& b)
{
	m_simd = _mm_sub_ps(m_simd, b.m_simd);
	return static_cast<TVec4<F32>&>(*this);
}

//==============================================================================
template<>
inline TVec4<F32> TVec4<F32>::Base::operator*(const TVec4<F32>& b) const
{
	return TVec4<F32>(_mm_mul_ps(m_simd, b.m_simd));
}

//==============================================================================
template<>
inline TVec4<F32>& TVec4<F32>::Base::operator*=(const TVec4<F32>& b)
{
	m_simd = _mm_mul_ps(m_simd, b.m_simd);
	return static_cast<TVec4<F32>&>(*this);
}

//==============================================================================
template<>
inline TVec4<F32> TVec4<F32>::Base::operator/(const TVec4<F32>& b) const
{
	return TVec4<F32>(_mm_div_ps(m_simd, b.m_simd));
}

//==============================================================================
template<>
inline TVec4<F32>& TVec4<F32>::Base::operator/=(const TVec4<F32>& b)
{
	m_simd = _mm_div_ps(m_simd, b.m_simd);
	return static_cast<TVec4<F32>&>(*this);
}

//==============================================================================
// Other                                                                       =
//==============================================================================

//==============================================================================
template<>
inline F32 TVec4<F32>::Base::dot(const TVec4<F32>& b) const
{
	F32 o;
	_mm_store_ss(&o, _mm_dp_ps(m_simd, b.m_simd, 0xF1));
	return o;
}

//==============================================================================
template<>
inline TVec4<F32> TVec4<F32>::Base::getNormalized() const
{
	__m128 inverse_norm = _mm_rsqrt_ps(_mm_dp_ps(m_simd, m_simd, 0xFF));
	return TVec4<F32>(_mm_mul_ps(m_simd, inverse_norm));
}

//==============================================================================
template<>
inline void TVec4<F32>::Base::normalize()
{
	__m128 inverseNorm = _mm_rsqrt_ps(_mm_dp_ps(m_simd, m_simd, 0xFF));
	m_simd = _mm_mul_ps(m_simd, inverseNorm);
}

#elif ANKI_SIMD == ANKI_SIMD_NEON

//==============================================================================
// NEON specializations                                                        =
//==============================================================================

#	error "TODO"

#endif

} // end namespace anki
