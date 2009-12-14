#ifndef _M_MISC_H_
#define _M_MISC_H_

#include "common.h"
#include "forward_decls.h"


namespace m {


const float PI = 3.14159265358979323846;
const float EPSILON = 1.0e-6;


void  MathSanityChecks();
void  SinCos( float rad, float& sin_, float& cos_ );
float InvSqrt( float f );
float Sqrt( float f );
float ToRad( float degrees );
float ToDegrees( float rad );
float Sin( float rad );
float Cos( float rad );
bool  IsZero( float f );
float Max( float a, float b );
float Min( float a, float b );


/**
 * CombineTransformations
 * mat4(t0,r0,s0)*mat4(t1,r1,s1) == mat4(tf,rf,sf)
 */
void CombineTransformations( const vec3_t& t0, const mat3_t& r0, float s0,
                                    const vec3_t& t1, const mat3_t& r1, float s1,
                                    vec3_t& tf, mat3_t& rf, float& sf );

/// CombineTransformations as the above but without scale
void CombineTransformations( const vec3_t& t0, const mat3_t& r0, const vec3_t& t1, const mat3_t& r1, vec3_t& tf, mat3_t& rf);


} // end namespace


#include "m_misc.inl.h"


#endif
