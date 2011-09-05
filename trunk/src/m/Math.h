#ifndef MATH_H
#define MATH_H

#include "MathCommonIncludes.h"


/// @addtogroup Math
/// @{

/// Useful and optimized math functions
class Math
{
	public:
		static const float PI;
		static const float EPSILON;

		/// A fast func that given the angle in rads it returns the sin and cos
		static void sinCos(const float rad, float& sin_, float& cos_);

		/// Optimized square root
		static float sqrt(const float f);

		/// Convert
		static float toRad(const float degrees);

		/// Convert
		static float toDegrees(const float rad);

		/// Optimized sine
		static float sin(const float rad);

		/// Optimized cosine
		static float cos(const float rad);

		/// The proper way to test if a float is zero
		static bool isZero(const float f);

		/// Mat4(t0,r0,s0) * Mat4(t1, r1, s1) == Mat4(tf, rf, sf)
		static void combineTransformations(
			const Vec3& t0, const Mat3& r0, const float s0, // in 0
			const Vec3& t1, const Mat3& r1, const float s1, // in 1
			Vec3& tf, Mat3& rf, float& sf); // out

		/// Mat4(t0, r0, 1.0) * Mat4(t1, r1, 1.0) == Mat4(tf, rf, sf)
		static void combineTransformations(
			const Vec3& t0, const Mat3& r0, // in 0
			const Vec3& t1, const Mat3& r1, // in 1
			Vec3& tf, Mat3& rf); // out

	private:
		static float polynomialSinQuadrant(const float a);
};
/// @}


#include "Math.inl.h"


#endif
