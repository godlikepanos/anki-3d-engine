#ifndef MATH_FUNCS_H
#define MATH_FUNCS_H

#include "Common.h"
#include "MathForwardDecls.h"


namespace M {


const float PI = 3.14159265358979323846;
const float EPSILON = 1.0e-6;


extern void  mathSanityChecks(); ///< Used to test the compiler
extern void  sinCos(float rad, float& sin_, float& cos_); ///< A fast func that given the angle in rads it returns the sin and cos
extern float invSqrt(float f); ///< Inverted square root
extern float sqrt(float f); ///< Square root
extern float toRad(float degrees);
extern float toDegrees(float rad);
extern float sin(float rad);
extern float cos(float rad);
extern bool  isZero(float f); ///< The proper way to test if a float is zero

/// mat4(t0,r0,s0)*mat4(t1,r1,s1) == mat4(tf,rf,sf)
extern void combineTransformations(const Vec3& t0, const Mat3& r0, float s0, // in 0
                                    const Vec3& t1, const Mat3& r1, float s1, // in 1
                                    Vec3& tf, Mat3& rf, float& sf); // out

/// mat4(t0,r0, 1.0)*mat4(t1,r1, 1.0) == mat4(tf,rf,sf)
extern void combineTransformations(const Vec3& t0, const Mat3& r0, // in 0
                                    const Vec3& t1, const Mat3& r1, // in 1
                                    Vec3& tf, Mat3& rf); // out


} // end namespace


#include "MathFuncs.inl.h"


#endif
