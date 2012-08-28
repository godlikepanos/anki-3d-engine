#ifndef ANKI_MATH_MATH_H
#define ANKI_MATH_MATH_H

#include "anki/math/MathCommonIncludes.h"

namespace anki {

/// @addtogroup Math
/// @{

/// Useful and optimized math functions
class Math
{
public:
	static const F32 PI;
	static const F32 EPSILON;

	/// A fast func that given the angle in rads it returns the sin and cos
	static void sinCos(const F32 rad, F32& sin_, F32& cos_);

	/// Optimized square root
	static F32 sqrt(const F32 f);

	/// Convert
	static F32 toRad(const F32 degrees);

	/// Convert
	static F32 toDegrees(const F32 rad);

	/// Optimized sine
	static F32 sin(const F32 rad);

	/// Optimized cosine
	static F32 cos(const F32 rad);

	/// The proper way to test if a F32 is zero
	static Bool isZero(const F32 f);

	/// Mat4(t0,r0,s0) * Mat4(t1, r1, s1) == Mat4(tf, rf, sf)
	static void combineTransformations(
		const Vec3& t0, const Mat3& r0, const F32 s0, // in 0
		const Vec3& t1, const Mat3& r1, const F32 s1, // in 1
		Vec3& tf, Mat3& rf, F32& sf); // out

	/// Mat4(t0, r0, 1.0) * Mat4(t1, r1, 1.0) == Mat4(tf, rf, sf)
	static void combineTransformations(
		const Vec3& t0, const Mat3& r0, // in 0
		const Vec3& t1, const Mat3& r1, // in 1
		Vec3& tf, Mat3& rf); // out

private:
	static F32 polynomialSinQuadrant(const F32 a);
};
/// @}

} // end namespace

#include "anki/math/Math.inl.h"

#endif
