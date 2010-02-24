#include "m_dflt_header.h"


namespace m {


// mathSanityChecks
// test if the compiler keeps the correct sizes for the classes
inline void mathSanityChecks()
{
	const int fs = sizeof(float); // float size
	if( sizeof(vec2_t)!=fs*2 || sizeof(vec3_t)!=fs*3 || sizeof(vec4_t)!=fs*4 || sizeof(quat_t)!=fs*4 || sizeof(Euler)!=fs*3 ||
	    sizeof(mat3_t)!=fs*9 || sizeof(mat4_t)!=fs*16 )
		FATAL("Your compiler does class alignment. Quiting");
}


// 1/sqrt(f)
inline float invSqrt( float f )
{
#if defined( _DEBUG_ )
	return 1.0/sqrtf(f);
#elif defined( _PLATFORM_WIN_ )
	float fhalf = 0.5*f;
	int i = *(int*)&f;
	i = 0x5F3759DF - (i>>1);
	f = *(float*)&i;
	f *= 1.5 - fhalf*f*f;
	return f;
#elif defined( _PLATFORM_LINUX_ )
	float fhalf = 0.5*f;
	asm
	(
		"mov %1, %%eax;"
		"sar %%eax;"
		"mov $0x5F3759DF, %%ebx;"
		"sub %%eax, %%ebx;"
		"mov %%ebx, %0"
		:"=g"(f)
		:"g"(f)
		:"%eax", "%ebx"
	);
	f *= 1.5 - fhalf*f*f;
	return f;
#else
	#error "See file"
#endif
}


// PolynomialSinQuadrant
// used in sinCos
#if !defined(_DEBUG_)
inline static float PolynomialSinQuadrant(float a)
{
	return a * ( 1.0 + a * a * (-0.16666 + a * a * (0.0083143 - a * a * 0.00018542)));
}
#endif


// Sine and Cosine
inline void sinCos( float a, float& sina, float& cosa )
{
#ifdef _DEBUG_
	sina = sin(a);
	cosa = cos(a);
#else
	bool negative = false;
	if (a < 0.0)
	{
		a = -a;
		negative = true;
	}
	const float k_two_over_pi = 1.0 / (PI/2.0);
	float float_a = k_two_over_pi * a;
	int int_a = (int)float_a;

	const float k_rational_half_pi = 201 / 128.0;
	const float k_remainder_half_pi = 4.8382679e-4;

	float_a = (a - k_rational_half_pi * int_a) - k_remainder_half_pi * int_a;

	float float_a_minus_half_pi = (float_a - k_rational_half_pi) - k_remainder_half_pi;

	switch( int_a & 3 )
	{
	case 0: // 0 - Pi/2
		sina = PolynomialSinQuadrant(float_a);
		cosa = PolynomialSinQuadrant(-float_a_minus_half_pi);
		break;
	case 1: // Pi/2 - Pi
		sina = PolynomialSinQuadrant(-float_a_minus_half_pi);
		cosa = PolynomialSinQuadrant(-float_a);
		break;
	case 2: // Pi - 3Pi/2
		sina = PolynomialSinQuadrant(-float_a);
		cosa = PolynomialSinQuadrant(float_a_minus_half_pi);
		break;
	case 3: // 3Pi/2 - 2Pi
		sina = PolynomialSinQuadrant(float_a_minus_half_pi);
		cosa = PolynomialSinQuadrant(float_a);
		break;
	};

	if( negative )
		sina = -sina;
#endif
}


//=====================================================================================================================================
// Small funcs                                                                                                                        =
//=====================================================================================================================================
inline float sqrt( float f ) { return 1/invSqrt(f); }
inline float ToRad( float degrees ) { return degrees*(PI/180.0); }
inline float ToDegrees( float rad ) { return rad*(180.0/PI); }
inline float Sin( float rad ) { return sin(rad); }
inline float Cos( float rad ) { return cos(rad); }
inline bool  IsZero( float f ) { return ( fabs(f) < EPSILON ); }
inline float Max( float a, float b ) { return (a>b) ? a : b; }
inline float Min( float a, float b ) { return (a<b) ? a : b; }


//  CombineTransformations
//  mat4(t0,r0,s0)*mat4(t1,r1,s1) == mat4(tf,rf,sf)
inline void CombineTransformations( const vec3_t& t0, const mat3_t& r0, float s0,
                                      const vec3_t& t1, const mat3_t& r1, float s1,
                                      vec3_t& tf, mat3_t& rf, float& sf )
{
	tf = t1.GetTransformed( t0, r0, s0 );
	rf = r0 * r1;
	sf = s0 * s1;
}

//  CombineTransformations as the above but without scale
inline void CombineTransformations( const vec3_t& t0, const mat3_t& r0, const vec3_t& t1, const mat3_t& r1, vec3_t& tf, mat3_t& rf)
{
	tf = t1.GetTransformed( t0, r0 );
	rf = r0 * r1;
}

} // end namespace
