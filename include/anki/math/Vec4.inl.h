#include "anki/math/CommonSrc.h"

namespace anki {

//==============================================================================
// Friends                                                                     =
//==============================================================================

//==============================================================================
template<typename T>
TVec4<T> operator+(const T f, const TVec4<T>& v4)
{
	return v4 + f;
}

//==============================================================================
template<typename T>
TVec4<T> operator-(const T f, const TVec4<T>& v4)
{
	return TVec4<T>(f) - v4;
}

//==============================================================================
template<typename T>
TVec4<T> operator*(const T f, const TVec4<T>& v4)
{
	return v4 * f;
}

//==============================================================================
template<typename T>
TVec4<T> operator/(const T f, const TVec4<T>& v4)
{
	return TVec4<T>(f) / v4;
}

#if ANKI_MATH_SIMD == ANKI_MATH_SIMD_SSE

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
	simd = _mm_set1_ps(f);
}

//==============================================================================
template<>
inline TVec4<F32>::TVec4(const F32 arr_[])
{
	simd = _mm_load_ps(arr_);
}

//==============================================================================
template<>
inline TVec4<F32>::TVec4(const F32 x_, const F32 y_, const F32 z_, const F32 w_)
{
	simd = _mm_set_ps(w_, z_, y_, x_);
}

//==============================================================================
template<>
inline TVec4<F32>::TVec4(const TVec4<F32>& b)
{
	simd = b.simd;
}

//==============================================================================
// Operators with same                                                         =
//==============================================================================

//==============================================================================
template<>
inline TVec4<F32>& TVec4<F32>::operator=(const TVec4<F32>& b)
{
	simd = b.simd;
	return (*this);
}

//==============================================================================
template<>
inline TVec4<F32> TVec4<F32>::operator+(const TVec4<F32>& b) const
{
	return TVec4<F32>(_mm_add_ps(simd, b.simd));
}

//==============================================================================
template<>
inline TVec4<F32>& TVec4<F32>::operator+=(const TVec4<F32>& b)
{
	simd = _mm_add_ps(simd, b.simd);
	return (*this);
}

//==============================================================================
template<>
inline TVec4<F32> TVec4<F32>::operator-(const TVec4<F32>& b) const
{
	return TVec4<F32>(_mm_sub_ps(simd, b.simd));
}

//==============================================================================
template<>
inline TVec4<F32>& TVec4<F32>::operator-=(const TVec4<F32>& b)
{
	simd = _mm_sub_ps(simd, b.simd);
	return (*this);
}

//==============================================================================
template<>
inline TVec4<F32> TVec4<F32>::operator*(const TVec4<F32>& b) const
{
	return TVec4<F32>(_mm_mul_ps(simd, b.simd));
}

//==============================================================================
template<>
inline TVec4<F32>& TVec4<F32>::operator*=(const TVec4<F32>& b)
{
	simd = _mm_mul_ps(simd, b.simd);
	return (*this);
}

//==============================================================================
template<>
inline TVec4<F32> TVec4<F32>::operator/(const TVec4<F32>& b) const
{
	return TVec4<F32>(_mm_div_ps(simd, b.simd));
}

//==============================================================================
template<>
inline TVec4<F32>& TVec4<F32>::operator/=(const TVec4<F32>& b)
{
	simd = _mm_div_ps(simd, b.simd);
	return (*this);
}

//==============================================================================
// Other                                                                       =
//==============================================================================

//==============================================================================
template<>
inline F32 TVec4<F32>::dot(const TVec4<F32>& b) const
{
	F32 o;
	_mm_store_ss(&o, _mm_dp_ps(simd, b.simd, 0xF1));
	return o;
}

//==============================================================================
template<>
inline TVec4<F32> TVec4<F32>::getNormalized() const
{
	__m128 inverse_norm = _mm_rsqrt_ps(_mm_dp_ps(simd, simd, 0xFF));
	return TVec4<F32>(_mm_mul_ps(simd, inverse_norm));
}

//==============================================================================
template<>
inline void TVec4<F32>::normalize()
{
	__m128 inverseNorm = _mm_rsqrt_ps(_mm_dp_ps(simd, simd, 0xFF));
	simd = _mm_mul_ps(simd, inverseNorm);
}

#elif ANKI_MATH_SIMD == ANKI_MATH_SIMD_NEON

//==============================================================================
// NEON specializations                                                        =
//==============================================================================

//==============================================================================
// Constructors                                                                =
//==============================================================================

//==============================================================================
template<>
inline TVec4<F32>::TVec4(F32 f)
{
	simd = vdupq_n_f32(f);
}

//==============================================================================
template<>
inline TVec4<F32>::TVec4(const F32 arr_[])
{
	simd = vld1q_f32(arr_);
}

//==============================================================================
template<>
inline TVec4<F32>::TVec4(const TVec4<F32>& b)
{
	simd = b.simd;
}

//==============================================================================
// Operators with same                                                         =
//==============================================================================

//==============================================================================
template<>
inline TVec4<F32>& TVec4<F32>::operator=(const TVec4<F32>& b)
{
	simd = b.simd;
	return (*this);
}

//==============================================================================
template<>
inline TVec4<F32> TVec4<F32>::operator+(const TVec4<F32>& b) const
{
	return TVec4<F32>(vaddq_f32(simd, b.simd));
}

//==============================================================================
template<>
inline TVec4<F32>& TVec4<F32>::operator+=(const TVec4<F32>& b)
{
	simd = vaddq_f32(simd, b.simd);
	return (*this);
}

//==============================================================================
template<>
inline TVec4<F32> TVec4<F32>::operator-(const TVec4<F32>& b) const
{
	return TVec4<F32>(vsubq_f32(simd, b.simd));
}

//==============================================================================
template<>
inline TVec4<F32>& TVec4<F32>::operator-=(const TVec4<F32>& b)
{
	simd = vsubq_f32(simd, b.simd);
	return (*this);
}

//==============================================================================
template<>
inline TVec4<F32> TVec4<F32>::operator*(const TVec4<F32>& b) const
{
	return TVec4<F32>(vmulq_f32(simd, b.simd));
}

//==============================================================================
template<>
inline TVec4<F32>& TVec4<F32>::operator*=(const TVec4<F32>& b)
{
	simd = vmulq_f32(simd, b.simd);
	return (*this);
}

//==============================================================================
// Operators with F32                                                          =
//==============================================================================

//==============================================================================
template<>
inline TVec4<F32> operator*(const F32 f) const
{
	return TVec4<F32>(vmulq_n_f32(simd, f));
}

//==============================================================================
template<>
inline TVec4<F32>& operator*=(const F32 f)
{
	simd = vmulq_n_f32(simd, f);
	return *this;
}

//==============================================================================
// Other                                                                       =
//==============================================================================

//==============================================================================
template<>
inline F32 TVec4<F32>::dot(const TVec4<F32>& b) const
{
	F32 o;
	// XXX
	return o;
}

//==============================================================================
template<>
inline TVec4<F32> TVec4<F32>::getNormalized() const
{
	// XXX
}

//==============================================================================
template<>
inline void TVec4<F32>::normalize()
{
	// XXX
}

#endif

} // end namespace anki
