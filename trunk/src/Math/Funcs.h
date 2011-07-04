#ifndef M_FUNCS_H
#define M_FUNCS_H

#include "Common.h"


namespace M {


const float PI = 3.14159265358979323846;
const float EPSILON = 1.0e-6;

/// A fast func that given the angle in rads it returns the sin and cos
void  sinCos(float rad, float& sin_, float& cos_);
float sqrt(float f); ///< Square root
float toRad(float degrees);
float toDegrees(float rad);
float sin(float rad);
float cos(float rad);
bool  isZero(float f); ///< The proper way to test if a float is zero

/// mat4(t0,r0,s0)*mat4(t1,r1,s1) == mat4(tf,rf,sf)
void combineTransformations(
	const Vec3& t0, const Mat3& r0, float s0, // in 0
	const Vec3& t1, const Mat3& r1, float s1, // in 1
	Vec3& tf, Mat3& rf, float& sf); // out

/// mat4(t0,r0, 1.0)*mat4(t1,r1, 1.0) == mat4(tf,rf,sf)
void combineTransformations(
	const Vec3& t0, const Mat3& r0, // in 0
	const Vec3& t1, const Mat3& r1, // in 1
	Vec3& tf, Mat3& rf); // out


} // end namespace


#include "Funcs.inl.h"


#endif
