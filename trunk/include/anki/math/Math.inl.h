#include "anki/math/MathCommonSrc.h"

namespace anki {

//==============================================================================
inline F32 Math::sqrt(const F32 f)
{
#if defined(ANKI_MATH_INTEL_SIMD)
	__m128 mm = _mm_set_ss(f);
	mm = _mm_sqrt_ss(mm);
	F32 o;
	_mm_store_ss(&o, mm);
	return o;
#else
	return ::sqrtf(f);
#endif
}

//==============================================================================
inline F32 Math::toRad(const F32 degrees)
{
	return degrees * (Math::PI / 180.0);
}

//==============================================================================
inline F32 Math::toDegrees(const F32 rad)
{
	return rad * (180.0 / Math::PI);
}

//==============================================================================
inline F32 Math::sin(const F32 rad)
{
	return ::sin(rad);
}

//==============================================================================
inline F32 Math::cos(const F32 rad)
{
	return ::cos(rad);
}

//==============================================================================
inline Bool Math::isZero(const F32 f)
{
	return fabs(f) < EPSILON;
}

//==============================================================================
inline void Math::combineTransformations(
	const Vec3& t0, const Mat3& r0, const F32 s0,
	const Vec3& t1, const Mat3& r1, const F32 s1,
	Vec3& tf, Mat3& rf, F32& sf)
{
	tf = t1.getTransformed(t0, r0, s0);
	rf = r0 * r1;
	sf = s0 * s1;
}

//==============================================================================
inline void Math::combineTransformations(
	const Vec3& t0, const Mat3& r0,
	const Vec3& t1, const Mat3& r1,
	Vec3& tf, Mat3& rf)
{
	tf = t1.getTransformed(t0, r0);
	rf = r0 * r1;
}

} // end namespace
