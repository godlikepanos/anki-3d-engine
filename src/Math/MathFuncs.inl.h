#include "MathDfltHeader.h"


namespace M {


// mathSanityChecks
// test if the compiler keeps the correct sizes for the classes
inline void mathSanityChecks()
{
	const int fs = sizeof(float); // float size
	if( sizeof(Vec2)!=fs*2 || sizeof(Vec3)!=fs*3 || sizeof(Vec4)!=fs*4 || sizeof(Quat)!=fs*4 || sizeof(Euler)!=fs*3 ||
	    sizeof(Mat3)!=fs*9 || sizeof(Mat4)!=fs*16 )
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
	sina = M::sin(a);
	cosa = M::cos(a);
#else
	bool negative = false;
	if (a < 0.0)
	{
		a = -a;
		negative = true;
	}
	const float kTwoOverPi = 1.0 / (PI/2.0);
	float floatA = kTwoOverPi * a;
	int int_a = (int)floatA;

	const float k_rational_half_pi = 201 / 128.0;
	const float kRemainderHalfPi = 4.8382679e-4;

	floatA = (a - k_rational_half_pi * int_a) - kRemainderHalfPi * int_a;

	float floatAMinusHalfPi = (floatA - k_rational_half_pi) - kRemainderHalfPi;

	switch( int_a & 3 )
	{
	case 0: // 0 - Pi/2
		sina = PolynomialSinQuadrant(floatA);
		cosa = PolynomialSinQuadrant(-floatAMinusHalfPi);
		break;
	case 1: // Pi/2 - Pi
		sina = PolynomialSinQuadrant(-floatAMinusHalfPi);
		cosa = PolynomialSinQuadrant(-floatA);
		break;
	case 2: // Pi - 3Pi/2
		sina = PolynomialSinQuadrant(-floatA);
		cosa = PolynomialSinQuadrant(floatAMinusHalfPi);
		break;
	case 3: // 3Pi/2 - 2Pi
		sina = PolynomialSinQuadrant(floatAMinusHalfPi);
		cosa = PolynomialSinQuadrant(floatA);
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
inline float toRad( float degrees ) { return degrees*(PI/180.0); }
inline float toDegrees( float rad ) { return rad*(180.0/PI); }
inline float sin( float rad ) { return ::sin(rad); }
inline float cos( float rad ) { return ::cos(rad); }
inline bool  isZero( float f ) { return ( fabs(f) < EPSILON ); }


//  combineTransformations
//  mat4(t0,r0,s0)*mat4(t1,r1,s1) == mat4(tf,rf,sf)
inline void combineTransformations( const Vec3& t0, const Mat3& r0, float s0,
                                    const Vec3& t1, const Mat3& r1, float s1,
                                    Vec3& tf, Mat3& rf, float& sf )
{
	tf = t1.getTransformed( t0, r0, s0 );
	rf = r0 * r1;
	sf = s0 * s1;
}

//  combineTransformations as the above but without scale
inline void combineTransformations( const Vec3& t0, const Mat3& r0, const Vec3& t1, const Mat3& r1, Vec3& tf, Mat3& rf)
{
	tf = t1.getTransformed( t0, r0 );
	rf = r0 * r1;
}

} // end namespace
