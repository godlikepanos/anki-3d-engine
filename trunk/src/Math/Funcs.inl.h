#include "Common.inl.h"


namespace M {


//==============================================================================
// Small funcs                                                                 =
//==============================================================================
inline float sqrt(float f)
{
	#if defined(MATH_INTEL_SIMD)
		__m128 mm = _mm_set_ss(f);
		mm = _mm_sqrt_ss(mm);
		float o;
		_mm_store_ss(&o, mm);
		return o;
	#else
		return ::sqrtf(f);
	#endif
}


inline float toRad(float degrees)
{
	return degrees * (PI / 180.0);
}


inline float toDegrees(float rad)
{
	return rad * (180.0 / PI);
}


inline float sin(float rad)
{
	return ::sin(rad);
}


inline float cos(float rad)
{
	return ::cos(rad);
}


inline bool isZero(float f)
{
	return fabs(f) < EPSILON;
}


//  combineTransformations
//  mat4(t0,r0,s0)*mat4(t1,r1,s1) == mat4(tf,rf,sf)
inline void combineTransformations(const Vec3& t0, const Mat3& r0, float s0,
                                   const Vec3& t1, const Mat3& r1, float s1,
                                   Vec3& tf, Mat3& rf, float& sf)
{
	tf = t1.getTransformed(t0, r0, s0);
	rf = r0 * r1;
	sf = s0 * s1;
}


//  combineTransformations as the above but without scale
inline void combineTransformations(const Vec3& t0, const Mat3& r0, const Vec3& t1, const Mat3& r1, Vec3& tf, Mat3& rf)
{
	tf = t1.getTransformed(t0, r0);
	rf = r0 * r1;
}


} // end namespace
