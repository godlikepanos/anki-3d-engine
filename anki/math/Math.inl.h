#include "anki/math/MathCommonSrc.h"


namespace anki {


inline float Math::sqrt(const float f)
{
#if defined(ANKI_MATH_INTEL_SIMD)
	__m128 mm = _mm_set_ss(f);
	mm = _mm_sqrt_ss(mm);
	float o;
	_mm_store_ss(&o, mm);
	return o;
#else
	return ::sqrtf(f);
#endif
}


inline float Math::toRad(const float degrees)
{
	return degrees * (Math::PI / 180.0);
}


inline float Math::toDegrees(const float rad)
{
	return rad * (180.0 / Math::PI);
}


inline float Math::sin(const float rad)
{
	return ::sin(rad);
}


inline float Math::cos(const float rad)
{
	return ::cos(rad);
}


inline bool Math::isZero(const float f)
{
	return fabs(f) < EPSILON;
}


inline void Math::combineTransformations(
	const Vec3& t0, const Mat3& r0, const float s0,
	const Vec3& t1, const Mat3& r1, const float s1,
	Vec3& tf, Mat3& rf, float& sf)
{
	tf = t1.getTransformed(t0, r0, s0);
	rf = r0 * r1;
	sf = s0 * s1;
}


inline void Math::combineTransformations(
	const Vec3& t0, const Mat3& r0,
	const Vec3& t1, const Mat3& r1,
	Vec3& tf, Mat3& rf)
{
	tf = t1.getTransformed(t0, r0);
	rf = r0 * r1;
}


} // end namespace
