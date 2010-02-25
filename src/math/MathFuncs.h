#ifndef _MATHFUNCS_H_
#define _MATHFUNCS_H_

#include "common.h"
#include "MathForwardDecls.h"


namespace M {


const float PI = 3.14159265358979323846;
const float EPSILON = 1.0e-6;


void  mathSanityChecks();
void  sinCos( float rad, float& sin_, float& cos_ );
float invSqrt( float f );
float sqrt( float f );
float toRad( float degrees );
float toDegrees( float rad );
float sin( float rad );
float cos( float rad );
bool  isZero( float f );

/**
 * combineTransformations
 * mat4(t0,r0,s0)*mat4(t1,r1,s1) == mat4(tf,rf,sf)
 */
void combineTransformations( const Vec3& t0, const Mat3& r0, float s0,
                             const Vec3& t1, const Mat3& r1, float s1,
                             Vec3& tf, Mat3& rf, float& sf );

/// combineTransformations as the above but without scale
void combineTransformations( const Vec3& t0, const Mat3& r0, const Vec3& t1, const Mat3& r1, Vec3& tf, Mat3& rf);


} // end namespace


#include "MathFuncs.inl.h"


#endif
