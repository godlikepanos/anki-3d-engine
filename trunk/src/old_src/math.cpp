#include "math.h"

#define ME (*this)


/**
=======================================================================================================================================
misc                                                                                                                                  =
=======================================================================================================================================
*/

// MathSanityChecks
// test if the compiler keeps the correct sizes for the classes
void MathSanityChecks()
{
	const int fs = sizeof(float); // float size
	if( sizeof(vec2_t)!=fs*2 || sizeof(vec3_t)!=fs*3 || sizeof(vec4_t)!=fs*4 || sizeof(quat_t)!=fs*4 || sizeof(euler_t)!=fs*3 ||
	    sizeof(mat3_t)!=fs*9 || sizeof(mat4_t)!=fs*16 )
		FATAL("Your compiler does class alignment. Quiting");
}


// 1/sqrt(f)
float InvSqrt( float f )
{
#ifdef WIN32
	float fhalf = 0.5f*f;
	int i = *(int*)&f;
	i = 0x5F3759DF - (i>>1);
	f = *(float*)&i;
	f *= 1.5f - fhalf*f*f;
	return f;
#elif defined( _DEBUG_ )
	return 1/sqrtf(f);
#else
	float fhalf = 0.5f*f;
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
	f *= 1.5f - fhalf*f*f;
	return f;
#endif
}


// PolynomialSinQuadrant
// used in SinCos
#if !defined(_DEBUG_)
static float PolynomialSinQuadrant(float a)
{
	return a * ( 1.0f + a * a * (-0.16666f + a * a * (0.0083143f - a * a * 0.00018542f)));
}
#endif


// Sine and Cosine
void SinCos( float a, float& sina, float& cosa )
{
#ifdef _DEBUG_
	sina = sin(a);
	cosa = cos(a);
#else
	bool negative = false;
	if (a < 0.0f)
	{
		a = -a;
		negative = true;
	}
	const float k_two_over_pi = 1.0f / (PI/2.0f);
	float float_a = k_two_over_pi * a;
	int int_a = (int)float_a;

	const float k_rational_half_pi = 201 / 128.0f;
	const float k_remainder_half_pi = 4.8382679e-4f;

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


/*
=======================================================================================================================================
vec2_t                                                                                                                                =
=======================================================================================================================================
*/
vec2_t vec2_t::zero = vec2_t(0.0, 0.0);

// constructor [vec3]
vec2_t::vec2_t( const vec3_t& v )
{
	x = v.x;
	y = v.y;
}

// constructor [vec4]
vec2_t::vec2_t( const vec4_t& v )
{
	x = v.x;
	y = v.y;
}

// copy
vec2_t& vec2_t::operator =( const vec2_t & b )
{
	x = b.x;
	y = b.y;
	return ME;
}

// +
vec2_t vec2_t::operator +( const vec2_t& b ) const
{
	return vec2_t( x+b.x, y+b.y );
}

// +=
vec2_t& vec2_t::operator +=( const vec2_t& b )
{
	x += b.x;
	y += b.y;
	return ME;
}

// -
vec2_t vec2_t::operator -( const vec2_t& b ) const
{
	return vec2_t( x-b.x, y-b.y );
}

// -=
vec2_t& vec2_t::operator -=( const vec2_t& b )
{
	x -= b.x;
	y -= b.y;
	return ME;
}

// negative
vec2_t vec2_t::operator -() const
{
	return vec2_t( -x, -y );
}

// *
vec2_t vec2_t::operator *( const vec2_t& b ) const
{
	return vec2_t( x*b.x, y*b.y );
}

// *=
vec2_t& vec2_t::operator *=( const vec2_t& b )
{
	x *= b.x;
	y *= b.y;
	return ME;
}

// /
vec2_t vec2_t::operator /( const vec2_t& b ) const
{
	return vec2_t( x/b.x, y/b.y );
}

// /=
vec2_t& vec2_t::operator /=( const vec2_t& b )
{
	x /= b.x;
	y /= b.y;
	return ME;
}

// scale
vec2_t vec2_t::operator *( float f ) const
{
	return vec2_t( x*f, y*f );
}

// scale
vec2_t& vec2_t::operator *=( float f )
{
	x *= f;
	y *= f;
	return ME;
}

// scale
vec2_t vec2_t::operator /( float f ) const
{
	DEBUG_ERR( IsZero(f) ); // division by zero
	return vec2_t( x/f,  y/f );
}

// scale
vec2_t& vec2_t::operator /=( float f )
{
	DEBUG_ERR( IsZero(f) ); // division by zero
	x /= f;
	y /= f;
	return ME;
}

// ==
bool vec2_t::operator ==( const vec2_t& b ) const
{
	return ( IsZero(x-b.x) && IsZero(y-b.y) ) ? true : false;
}

// !=
bool vec2_t::operator !=( const vec2_t& b ) const
{
	return ( IsZero(x-b.x) && IsZero(y-b.y) ) ? false : true;
}

// Length
float vec2_t::Length() const
{
	return Sqrt( x*x + y*y );
}

// set to zero
void vec2_t::SetZero()
{
	x = y = 0.0f;
}

// Normalize
void vec2_t::Normalize()
{
	ME *= InvSqrt( x*x + y*y );
}

// Normalized (return the normalized)
vec2_t vec2_t::Normalized() const
{
	return ME * InvSqrt( x*x + y*y );
}

// Dot
float vec2_t::Dot( const vec2_t& b ) const
{
	return x*b.x + y*b.y;
}

// Print
void vec2_t::Print() const
{
	for( int i=0; i<2; i++ )
		cout << fixed << ME[i] << ' ';
	cout << "\n" << endl;
}


/*
=======================================================================================================================================
vec3_t                                                                                                                                =
=======================================================================================================================================
*/
vec3_t vec3_t::zero(0.0f, 0.0f, 0.0f);
vec3_t vec3_t::one(1.0f, 1.0f, 1.0f);
vec3_t vec3_t::i(1.0f, 0.0f, 0.0f);
vec3_t vec3_t::j(0.0f, 1.0f, 0.0f);
vec3_t vec3_t::k(0.0f, 0.0f, 1.0f);

// constructor [vec4]
vec3_t::vec3_t( const vec4_t& v4 )
{
	x = v4.x;
	y = v4.y;
	z = v4.z;
}

// constructor [quaternion]
vec3_t::vec3_t( const quat_t& q )
{
	x = q.x;
	y = q.y;
	z = q.z;
}

// copy
vec3_t& vec3_t::operator =( const vec3_t& b )
{
	x = b.x;
	y = b.y;
	z = b.z;
	return ME;
}

// +
vec3_t vec3_t::operator +( const vec3_t& b ) const
{
	return vec3_t( x+b.x, y+b.y, z+b.z );
}

// +=
vec3_t& vec3_t::operator +=( const vec3_t& b )
{
	x += b.x;
	y += b.y;
	z += b.z;
	return ME;
}

// -
vec3_t vec3_t::operator -( const vec3_t& b ) const
{
	return vec3_t( x-b.x, y-b.y, z-b.z );
}

// -=
vec3_t& vec3_t::operator -=( const vec3_t& b )
{
	x -= b.x;
	y -= b.y;
	z -= b.z;
	return ME;
}

// *
vec3_t vec3_t::operator *( const vec3_t& b ) const
{
	return vec3_t( x*b.x, y*b.y, z*b.z );
}

// *=
vec3_t& vec3_t::operator *=( const vec3_t& b )
{
	x *= b.x;
	y *= b.y;
	z *= b.z;
	return ME;
}

// /
vec3_t vec3_t::operator /( const vec3_t& b ) const
{
	return vec3_t( x/b.x, y/b.y, z/b.z );
}

// /=
vec3_t& vec3_t::operator /=( const vec3_t& b )
{
	x /= b.x;
	y /= b.y;
	z /= b.z;
	return ME;
}

// negative
vec3_t vec3_t::operator -() const
{
	return vec3_t( -x, -y, -z );
}

// scale
vec3_t vec3_t::operator *( float f ) const
{
	return vec3_t( x*f, y*f, z*f );
}

// scale
vec3_t& vec3_t::operator *=( float f )
{
	x*=f;  y*=f;  z*=f;
	return ME;
}

// down-scale
vec3_t vec3_t::operator /( float f ) const
{
	DEBUG_ERR( IsZero(f) ); // division by zero
	return vec3_t( x/f, y/f, z/f );
}

// down-scale
vec3_t& vec3_t::operator /=( float f )
{
	DEBUG_ERR( IsZero(f) ); // division by zero
	x /= f;
	y /= f;
	z /= f;
	return ME;
}

// vec3 * mat3
vec3_t vec3_t::operator *( const mat3_t& m3 ) const
{
	return vec3_t(
		x*m3(0,0) + y*m3(1,0) + z*m3(2,0),
		x*m3(0,1) + y*m3(1,1) + z*m3(2,1),
		x*m3(0,2) + y*m3(1,2) + z*m3(2,2)
	);
}

// ==
bool vec3_t::operator ==( const vec3_t& b ) const
{
	return ( IsZero(x-b.x) && IsZero(y-b.y) && IsZero(z-b.z) ) ? true : false;
}

// !=
bool vec3_t::operator !=( const vec3_t& b ) const
{
	return ( IsZero(x-b.x) && IsZero(y-b.y) && IsZero(z-b.z) ) ? false : true;
}

// Dot
float vec3_t::Dot( const vec3_t& b ) const
{
	return x*b.x + y*b.y + z*b.z;
}

// cross prod
vec3_t vec3_t::Cross( const vec3_t& b ) const
{
	return vec3_t( y*b.z-z*b.y, z*b.x-x*b.z, x*b.y-y*b.x );
}

// Length
float vec3_t::Length() const
{
	return Sqrt( x*x + y*y + z*z );
}

// LengthSquared
float vec3_t::LengthSquared() const
{
	return x*x + y*y + z*z;
}

// Normalize
void vec3_t::Normalize()
{
	ME *= InvSqrt( x*x + y*y + z*z );
}

// Normalized (return the normalized)
vec3_t vec3_t::Normalized() const
{
	return ME * InvSqrt( x*x + y*y + z*z );
}

// Project
vec3_t vec3_t::Project( const vec3_t& to_this ) const
{
	return to_this * ( ME.Dot(to_this)/(to_this.Dot(to_this)) );
}

// Rotated
vec3_t vec3_t::Rotated( const quat_t& q ) const
{
	DEBUG_ERR( !IsZero(1.0f-q.Length()) ); // Not normalized quat

	/* Old code:
	float vmult = 2.0f*(q.x*x + q.y*y + q.z*z);
	float crossmult = 2.0*q.w;
	float pmult = crossmult*q.w - 1.0;

	return vec3_t( pmult*x + vmult*q.x + crossmult*(q.y*z - q.z*y),
							   pmult*y + vmult*q.y + crossmult*(q.z*x - q.x*z),
	               pmult*z + vmult*q.z + crossmult*(q.x*y - q.y*x) );*/

	vec3_t q_xyz = vec3_t( q );
	return ME + q_xyz.Cross( q_xyz.Cross( ME ) + ME*q.w ) * 2.0;
}

// Rotate
void vec3_t::Rotate( const quat_t& q )
{
	ME = Rotated(q);
}

// Print
void vec3_t::Print() const
{
	for( int i=0; i<3; i++ )
		cout << fixed << ME[i] << " ";
	cout << "\n" << endl;
}

// Lerp
vec3_t vec3_t::Lerp( const vec3_t& v1, float t ) const
{
	return (ME*(1.0f-t))+(v1*t);
}

// Transformed [mat3]
vec3_t vec3_t::Transformed( const vec3_t& translate, const mat3_t& rotate, float scale ) const
{
	return (rotate * (ME * scale)) + translate;
}

// Transformed [mat3] no scale
vec3_t vec3_t::Transformed( const vec3_t& translate, const mat3_t& rotate ) const
{
	return (rotate * ME) + translate;
}

// Transformed [quat]
vec3_t vec3_t::Transformed( const vec3_t& translate, const quat_t& rotate, float scale ) const
{
	return (ME * scale).Rotated(rotate) + translate;
}


/*
=======================================================================================================================================
vec4_t                                                                                                                                =
=======================================================================================================================================
*/
vec4_t vec4_t::zero = vec4_t(0.0, 0.0, 0.0, 0.0);
vec4_t vec4_t::one = vec4_t(1.0, 1.0, 1.0, 1.0);
vec4_t vec4_t::i = vec4_t(1.0, 0.0, 0.0, 0.0);
vec4_t vec4_t::j = vec4_t(0.0, 1.0, 0.0, 0.0);
vec4_t vec4_t::k = vec4_t(0.0, 0.0, 1.0, 0.0);
vec4_t vec4_t::l = vec4_t(0.0, 0.0, 0.0, 1.0);

// copy
vec4_t& vec4_t::operator =( const vec4_t& b )
{
	x = b.x;
	y = b.y;
	z = b.z;
	w = b.w;
	return ME;
}

// +
vec4_t vec4_t::operator +( const vec4_t& b ) const
{
	return vec4_t( x+b.x, y+b.y, z+b.z, w+b.w );
}

// +=
vec4_t& vec4_t::operator +=( const vec4_t& b )
{
	x += b.x;
	y += b.y;
	z += b.z;
	w += b.w;
	return ME;
}

// -
vec4_t vec4_t::operator -( const vec4_t& b ) const
{
	return vec4_t( x-b.x, y-b.y, z-b.z, w-b.w );
}

// -=
vec4_t& vec4_t::operator -=( const vec4_t& b )
{
	x -= b.x;
	y -= b.y;
	z -= b.z;
	w -= b.w;
	return ME;
}

// negative
vec4_t vec4_t::operator -() const
{
	return vec4_t( -x, -y, -z, -w );
}

// *
vec4_t vec4_t::operator *( const vec4_t& b ) const
{
	return vec4_t( x*b.x, y*b.y, z*b.z, w*b.w );
}

// *=
vec4_t& vec4_t::operator *=( const vec4_t& b )
{
	x *= b.x;
	y *= b.y;
	z *= b.z;
	w *= b.w;
	return ME;
}

// /
vec4_t vec4_t::operator /( const vec4_t& b ) const
{
	return vec4_t( x/b.x, y/b.y, z/b.z, w/b.w );
}

// /=
vec4_t& vec4_t::operator /=( const vec4_t& b )
{
	x /= b.x;
	y /= b.y;
	z /= b.z;
	w /= b.w;
	return ME;
}

// scale
vec4_t vec4_t::operator *( float f ) const
{
	return vec4_t( x*f, y*f, z*f, w*f );
}

// scale
vec4_t& vec4_t::operator *=( float f )
{
	x *= f;
	y *= f;
	z *= f;
	w *= f;
	return ME;
}

// down-scale
vec4_t vec4_t::operator /( float f ) const
{
	DEBUG_ERR( IsZero(f) ); // Division by zero
	return vec4_t( x/f, y/f, z/f, w/f );
}

// down-scale
vec4_t& vec4_t::operator /=( float f )
{
	DEBUG_ERR( IsZero(f) ); // Division by zero
	x /= f;
	y /= f;
	z /= f;
	w /= f;
	return ME;
}

// vec4 * mat4
vec4_t vec4_t::operator *( const mat4_t& m4 ) const
{
	return vec4_t(
		x*m4(0,0) + y*m4(1,0) + z*m4(2,0) + w*m4(3,0),
		x*m4(0,1) + y*m4(1,1) + z*m4(2,1) + w*m4(3,1),
		x*m4(0,2) + y*m4(1,2) + z*m4(2,2) + w*m4(3,2),
		x*m4(0,3) + y*m4(1,3) + z*m4(2,3) + w*m4(3,3)
	);
}

// ==
bool vec4_t::operator ==( const vec4_t& b ) const
{
	return ( IsZero(x-b.x) && IsZero(y-b.y) && IsZero(z-b.z) && IsZero(w-b.w) ) ? true : false;
}

// !=
bool vec4_t::operator !=( const vec4_t& b ) const
{
	return ( IsZero(x-b.x) && IsZero(y-b.y) && IsZero(z-b.z) && IsZero(w-b.w) ) ? false : true;
}

// Dot
float vec4_t::Dot( const vec4_t& b ) const
{
	return x*b.x + y*b.y + z*b.z + w*b.w;
}

// Length
float vec4_t::Length() const
{
	return Sqrt( x*x + y*y + z*z + w*w );
}

// Normalize
void vec4_t::Normalize()
{
	ME *= InvSqrt( x*x +y*y + z*z + w*w );
}

// SetZero
void vec4_t::SetZero()
{
	w = z = y = x = 0.0;
}

// Print
void vec4_t::Print() const
{
	for( int i=0; i<4; i++ )
		cout << fixed << ME[i] << " ";
	cout << "\n" << endl;
}


/*
=======================================================================================================================================
quat_t                                                                                                                                =
=======================================================================================================================================
*/
quat_t quat_t::zero = quat_t( 0.0, 0.0, 0.0, 0.0 );
quat_t quat_t::ident = quat_t( 1.0, 0.0, 0.0, 0.0 );


// copy
quat_t& quat_t::operator =( const quat_t& b )
{
	w = b.w;
	x = b.x;
	y = b.y;
	z = b.z;
	return ME;
}

// +
quat_t quat_t::operator +( const quat_t& b ) const
{
	return quat_t( w+b.w, x+b.x, y+b.y, z+b.z );
}

// +=
quat_t& quat_t::operator +=( const quat_t& b )
{
	w += b.w;
	x += b.x;
	y += b.y;
	z += b.z;
	return ME;
}

// -
quat_t quat_t::operator -( const quat_t& b ) const
{
	return quat_t( w-b.w, x-b.x, y-b.y, z-b.z );
}

// -=
quat_t& quat_t::operator -=( const quat_t& b )
{
	w -= b.w;
	x -= b.x;
	y -= b.y;
	z -= b.z;
	return ME;
}

// *
quat_t quat_t::operator *( const quat_t& b ) const
{
	return quat_t(
		-x * b.x - y * b.y - z * b.z + w * b.w,
		 x * b.w + y * b.z - z * b.y + w * b.x,
		-x * b.z + y * b.w + z * b.x + w * b.y,
		 x * b.y - y * b.x + z * b.w + w * b.z
	);
}

// *=
quat_t& quat_t::operator *=( const quat_t& b )
{
	ME = ME*b;
	return ME;
}

// quat * float
quat_t quat_t::operator *( float f ) const
{
	return quat_t( w*f, x*f,y*f, z*f );
}

// quat * float (self)
quat_t& quat_t::operator *=( float f )
{
	w *= f;
	x *= f;
	y *= f;
	z *= f;
	return ME;
}

// quat / float
quat_t quat_t::operator /( float f ) const
{
	DEBUG_ERR( IsZero(f) ); // Division by zero
	return quat_t( w/f, x/f, y/f, z/f );
}

// quat / float (self)
quat_t& quat_t::operator /=( float f )
{
	DEBUG_ERR( IsZero(f) ); // Division by zero
	w /= f;
	x /= f;
	y /= f;
	z /= f;
	return ME;
}

// ==
bool quat_t::operator ==( const quat_t& b ) const
{
	return ( IsZero(w-b.w) && IsZero(x-b.x) && IsZero(y-b.y) && IsZero(z-b.z) ) ? true : false;
}

// !=
bool quat_t::operator !=( const quat_t& b ) const
{
	return ( IsZero(w-b.w) && IsZero(x-b.x) && IsZero(y-b.y) && IsZero(z-b.z) ) ? false : true;
}

// Conjugate
void quat_t::Conjugate()
{
	x = -x;
	y = -y;
	z = -z;
}

// Conjugated
quat_t quat_t::Conjugated() const
{
	return quat_t( w, -x, -y, -z );
}

// Normalize
void quat_t::Normalize()
{
	ME *= InvSqrt( x*x + y*y + z*z + w*w );
}

// SetIdent
void quat_t::SetIdent()
{
	ME = ident;
}

// SetZero
void quat_t::SetZero()
{
	z = y = x = w = 0.0;
}

// Length
float quat_t::Length() const
{
	return Sqrt( w*w + x*x + y*y + z*z );
}

// Invert
void quat_t::Invert()
{
	float norm = w*w + x*x + y*y + z*z;

	DEBUG_ERR( IsZero(norm) ); // Norm is zero

	float normi = 1.0f / norm;
	ME = quat_t( normi*w, -normi*x, -normi*y, -normi*z );
}

// Print
void quat_t::Print() const
{
	cout << fixed << "(w,x,y,z) = " << w << ' ' << x << ' ' << y << ' ' << z  << '\n' << endl;
}

// 3x3 to quat
void quat_t::Set( const mat3_t& a )
{
	float trace = a(0, 0) + a(1, 1) + a(2, 2) + 1.0f;
	if( trace > EPSILON )
	{
		float s = 0.5f * InvSqrt(trace);
		w = 0.25f / s;
		x = ( a(2, 1) - a(1, 2) ) * s;
		y = ( a(0, 2) - a(2, 0) ) * s;
		z = ( a(1, 0) - a(0, 1) ) * s;
	}
	else
	{
		if( a(0, 0) > a(1, 1) && a(0, 0) > a(2, 2) )
		{
			float s = 2.0f * ( Sqrt( 1.0f + a(0, 0) - a(1, 1) - a(2, 2)) );
			w = (a(1, 2) - a(2, 1) ) / s;
			x = 0.25f * s;
			y = (a(0, 1) + a(1, 0) ) / s;
			z = (a(0, 2) + a(2, 0) ) / s;
		}
		else if( a(1, 1) > a(2, 2) )
		{
			float s = 2.0f * ( Sqrt( 1.0f + a(1, 1) - a(0, 0) - a(2, 2)) );
			w = (a(0, 2) - a(2, 0) ) / s;
			x = (a(0, 1) + a(1, 0) ) / s;
			y = 0.25f * s;
			z = (a(1, 2) + a(2, 1) ) / s;
		}
		else
		{
			float s = 2.0f * ( Sqrt( 1.0f + a(2, 2) - a(0, 0) - a(1, 1) ) );
			w = (a(0, 1) - a(1, 0) ) / s;
			x = (a(0, 2) + a(2, 0) ) / s;
			y = (a(1, 2) + a(2, 1) ) / s;
			z = 0.25f * s;
		}
	}

	Normalize();
}

// euler to quat
void quat_t::Set( const euler_t &e )
{
	float cx, sx;
	SinCos( e.heading()*0.5, sx, cx );

	float cy, sy;
	SinCos( e.attitude()*0.5, sy, cy );

	float cz, sz;
	SinCos( e.bank()*0.5, sz, cz );

	float cxcy = cx*cy;
	float sxsy = sx*sy;
	w = cxcy*cz - sxsy*sz;
	x = cxcy*sz + sxsy*cz;
	y = sx*cy*cz + cx*sy*sz;
	z = cx*sy*cz - sx*cy*sz;
}

// Set
// axis_ang
void quat_t::Set( const axisang_t& axisang )
{
	float lengthsq = axisang.axis.LengthSquared();
	if ( IsZero( lengthsq ) )
	{
		SetIdent();
		return;
	}

	float rad = axisang.ang * 0.5f;

	float sintheta, costheta;
	SinCos( rad, sintheta, costheta );

	float scalefactor = sintheta/ Sqrt(lengthsq);

	w = costheta;
	x = scalefactor * axisang.axis.x;
	y = scalefactor * axisang.axis.y;
	z = scalefactor * axisang.axis.z;
}

// CalcFromVecVec
void quat_t::CalcFromVecVec( const vec3_t& from, const vec3_t& to )
{
	vec3_t axis( from.Cross(to) );
	ME = quat_t(  from.Dot(to), axis.x, axis.y, axis.z);
	Normalize();
	w += 1.0f;

	if( w <= EPSILON )
	{
		if( from.z*from.z > from.x*from.x )
			ME = quat_t( 0.0f, 0.0f, from.z, -from.y );
		else
			ME = quat_t( 0.0f, from.y, -from.x, 0.0f );
	}
	Normalize();
}

// Set
// vec3
void quat_t::Set( const vec3_t& v )
{
	w = 0.0;
	x = v.x;
	y = v.y;
	z = v.y;
}

// Set
// vec4
void quat_t::Set( const vec4_t& v )
{
	w = v.w;
	x = v.x;
	y = v.y;
	z = v.y;
}

// Dot
float quat_t::Dot( const quat_t& b ) const
{
	return w*b.w + x*b.x + y*b.y + z*b.z;
}

// SLERP
quat_t quat_t::Slerp( const quat_t& q1_, float t ) const
{
	const quat_t& q0 = ME;
	quat_t q1( q1_ );
	float cos_half_theta = q0.w*q1.w + q0.x*q1.x + q0.y*q1.y + q0.z*q1.z;
	if( cos_half_theta < 0.0 )
	{
		q1 *= -1.0f; // quat changes
		cos_half_theta = -cos_half_theta;
	}
	if( fabs(cos_half_theta) >= 1.0f )
	{
		return quat_t( q0 );
	}

	float half_theta = acos( cos_half_theta );
	float sin_half_theta = Sqrt(1.0f - cos_half_theta*cos_half_theta);

	if( fabs(sin_half_theta) < 0.001f )
	{
		return quat_t( q0*0.5f + q1*0.5f );
	}
	float ratio_a = sin((1.0f - t) * half_theta) / sin_half_theta;
	float ratio_b = sin(t * half_theta) / sin_half_theta;
	quat_t tmp, tmp1, sum;
	tmp = q0*ratio_a;
	tmp1 = q1*ratio_b;
	sum = tmp + tmp1;
	sum.Normalize();
	return quat_t( sum );
}


/*
=======================================================================================================================================
euler_t                                                                                                                               =
=======================================================================================================================================
*/

// cpy
euler_t& euler_t::operator =( const euler_t& b )
{
	memcpy( this, &b, sizeof(euler_t) );
	return ME;
}

// LoadZero
void euler_t::SetZero()
{
	z = y = x = 0.0;
}

// Set
// quat
void euler_t::Set( const quat_t& q )
{
	float test = q.x*q.y + q.z*q.w;
	if (test > 0.499f)
	{
		heading() = 2.0f * atan2( q.x, q.w );
		attitude() = PI/2.0f;
		bank() = 0.0;
		return;
	}
	if (test < -0.499f)
	{
		heading() = -2.0f * atan2( q.x, q.w );
		attitude() = -PI/2.0f;
		bank() = 0.0;
		return;
	}

	float sqx = q.x*q.x;
	float sqy = q.y*q.y;
	float sqz = q.z*q.z;
	heading() = atan2( 2.0f*q.y*q.w-2.0f*q.x*q.z, 1.0f-2.0f*sqy-2.0f*sqz );
	attitude() = asin( 2.0f*test );
	bank() = atan2( 2.0f*q.x*q.w-2.0f*q.y*q.z, 1.0f-2.0f*sqx-2.0f*sqz );
}

// Set
// mat3
void euler_t::Set( const mat3_t& m3 )
{
	float cx, sx;
	float cy, sy;
	float cz, sz;

	sy = m3(0,2);
	cy = Sqrt( 1.0f - sy*sy );
	// normal case
	if ( !IsZero( cy ) )
	{
		float factor = 1.0f/cy;
		sx = -m3(1,2) * factor;
		cx = m3(2,2) * factor;
		sz = -m3(0,1) * factor;
		cz = m3(0,0) * factor;
	}
	// x and z axes aligned
	else
	{
		sz = 0.0f;
		cz = 1.0f;
		sx = m3(2,1);
		cx = m3(1,1);
	}

	z = atan2f( sz, cz );
	y = atan2f( sy, cy );
	x = atan2f( sx, cx );
}

// Print
void euler_t::Print() const
{
	for( int i=0; i<3; i++ )
		cout << fixed << ME[i] << "(" << ME[i]/PI << " PI) Rad" << " / " << ToDegrees(ME[i]) << " Deg" << "\n";
	cout << endl;
}


/*
=======================================================================================================================================
axisang_t                                                                                                                             =
=======================================================================================================================================
*/

// Set
// quat
void axisang_t::Set( const quat_t& q )
{
	ang = 2.0f*acos( q.w );
	float length = Sqrt( 1.0f - q.w*q.w );
	if( IsZero(length) )
		axis.SetZero();
	else
	{
		length = 1.0f/length;
		axis = vec3_t( q.x*length, q.y*length, q.z*length );
	}
}

// Set
// mat3
void axisang_t::Set( const mat3_t& m3 )
{
	if( (fabs(m3(0,1)-m3(1,0))< EPSILON)  && (fabs(m3(0,2)-m3(2,0))< EPSILON)  && (fabs(m3(1,2)-m3(2,1))< EPSILON) )
	{

		if( (fabs(m3(0,1)+m3(1,0)) < 0.1 ) && (fabs(m3(0,2)+m3(2,0)) < 0.1) && (fabs(m3(1,2)+m3(2,1)) < 0.1) && (fabs(m3(0,0)+m3(1,1)+m3(2,2))-3) < 0.1 )
		{
			axis = vec3_t( 1.0f, 0.0f, 0.0f );
			ang = 0.0f;
			return;
		}

		ang = PI;
		axis.x = (m3(0,0)+1)/2;
		if( axis.x > 0.0f )
			axis.x = Sqrt(axis.x);
		else
			axis.x = 0;
		axis.y = (m3(1,1)+1)/2;
		if( axis.y > 0 )
			axis.y = Sqrt(axis.y);
		else
			axis.y = 0;
		axis.z = (m3(2,2)+1)/2;
		if( axis.z > 0 )
			axis.z = Sqrt(axis.z);
		else
			axis.z = 0.0f;

		bool xZero = ( fabs(axis.x)<EPSILON );
		bool yZero = ( fabs(axis.y)<EPSILON );
		bool zZero = ( fabs(axis.z)<EPSILON );
		bool xyPositive = ( m3(0,1) > 0 );
		bool xzPositive = ( m3(0,2) > 0 );
		bool yzPositive = ( m3(1,2) > 0 );
		if( xZero && !yZero && !zZero ){
			if( !yzPositive ) axis.y = -axis.y;
		}else if( yZero && !zZero ){
			if( !xzPositive ) axis.z = -axis.z;
		}else if (zZero){
			if( !xyPositive ) axis.x = -axis.x;
		}

		return;
	}

	float s = Sqrt((m3(2,1) - m3(1,2))*(m3(2,1) - m3(1,2))+(m3(0,2) - m3(2,0))*(m3(0,2) - m3(2,0))+(m3(1,0) - m3(0,1))*(m3(1,0) - m3(0,1)));

	if( fabs(s) < 0.001 ) s = 1;

	ang = acos( ( m3(0,0) + m3(1,1) + m3(2,2) - 1)/2 );
	axis.x= (m3(2,1) - m3(1,2))/s;
	axis.y= (m3(0,2) - m3(2,0))/s;
	axis.z= (m3(1,0) - m3(0,1))/s;
}



/*
=======================================================================================================================================
matrix 3x3                                                                                                                            =
=======================================================================================================================================
*/
mat3_t mat3_t::ident( 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 );
mat3_t mat3_t::zero( 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 );


// constructor
mat3_t::mat3_t( float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22 )
{
	ME(0,0) = m00;
	ME(0,1) = m01;
	ME(0,2) = m02;
	ME(1,0) = m10;
	ME(1,1) = m11;
	ME(1,2) = m12;
	ME(2,0) = m20;
	ME(2,1) = m21;
	ME(2,2) = m22;
}


// copy
mat3_t& mat3_t::operator =( const mat3_t& b )
{
	ME[0] = b[0];
	ME[1] = b[1];
	ME[2] = b[2];
	ME[3] = b[3];
	ME[4] = b[4];
	ME[5] = b[5];
	ME[6] = b[6];
	ME[7] = b[7];
	ME[8] = b[8];
	return ME;
}

// 3x3 + 3x3
mat3_t mat3_t::operator +( const mat3_t& b ) const
{
	mat3_t c;
	for( int i=0; i<9; i++ )
		c[i] = ME[i] + b[i];
	return c;
}

// 3x3 + 3x3 (self)
mat3_t& mat3_t::operator +=( const mat3_t& b )
{
	for( int i=0; i<9; i++ )
		ME[i] += b[i];
	return ME;
}

// 3x3 - 3x3
mat3_t mat3_t::operator -( const mat3_t& b ) const
{
	mat3_t c;
	for( int i=0; i<9; i++ )
		c[i] = ME[i] - b[i];
	return c;
}

// 3x3 - 3x3 (self)
mat3_t& mat3_t::operator -=( const mat3_t& b )
{
	for( int i=0; i<9; i++ )
		ME[i] -= b[i];
	return ME;
}

// 3x3 * 3x3
mat3_t mat3_t::operator *( const mat3_t& b ) const
{
	mat3_t c;
	c(0, 0) = ME(0, 0)*b(0, 0) + ME(0, 1)*b(1, 0) + ME(0, 2)*b(2, 0);
	c(0, 1) = ME(0, 0)*b(0, 1) + ME(0, 1)*b(1, 1) + ME(0, 2)*b(2, 1);
	c(0, 2) = ME(0, 0)*b(0, 2) + ME(0, 1)*b(1, 2) + ME(0, 2)*b(2, 2);
	c(1, 0) = ME(1, 0)*b(0, 0) + ME(1, 1)*b(1, 0) + ME(1, 2)*b(2, 0);
	c(1, 1) = ME(1, 0)*b(0, 1) + ME(1, 1)*b(1, 1) + ME(1, 2)*b(2, 1);
	c(1, 2) = ME(1, 0)*b(0, 2) + ME(1, 1)*b(1, 2) + ME(1, 2)*b(2, 2);
	c(2, 0) = ME(2, 0)*b(0, 0) + ME(2, 1)*b(1, 0) + ME(2, 2)*b(2, 0);
	c(2, 1) = ME(2, 0)*b(0, 1) + ME(2, 1)*b(1, 1) + ME(2, 2)*b(2, 1);
	c(2, 2) = ME(2, 0)*b(0, 2) + ME(2, 1)*b(1, 2) + ME(2, 2)*b(2, 2);
	return c;
}

// 3x3 * 3x3 (self)
mat3_t& mat3_t::operator *=( const mat3_t& b )
{
	ME = ME * b;
	return ME;
}

// 3x3 * float
mat3_t mat3_t::operator *( float f ) const
{
	mat3_t c;
	for( uint i=0; i<9; i++ )
		c[i] = ME[i] * f;
	return c;
}

// 3x3 * float (self)
mat3_t& mat3_t::operator *=( float f )
{
	for( uint i=0; i<9; i++ )
		ME[i] *= f;
	return ME;
}

// 3x3 * vec3 (cross products with rows)
vec3_t mat3_t::operator *( const vec3_t& b ) const
{
	return vec3_t(
		ME(0, 0)*b.x + ME(0, 1)*b.y + ME(0, 2)*b.z,
		ME(1, 0)*b.x + ME(1, 1)*b.y + ME(1, 2)*b.z,
		ME(2, 0)*b.x + ME(2, 1)*b.y + ME(2, 2)*b.z
	);
}

// ==
bool mat3_t::operator ==( const mat3_t& b ) const
{
	for( int i=0; i<9; i++ )
		if( !IsZero( ME[i]-b[i] ) ) return false;
	return true;
}

// !=
bool mat3_t::operator !=( const mat3_t& b ) const
{
	for( int i=0; i<9; i++ )
		if( !IsZero( ME[i]-b[i] ) ) return true;
	return false;
}

// SetRows
void mat3_t::SetRows( const vec3_t& a, const vec3_t& b, const vec3_t& c )
{
	ME(0,0) = a.x;
	ME(0,1) = a.y;
	ME(0,2) = a.z;
	ME(1,0) = b.x;
	ME(1,1) = b.y;
	ME(1,2) = b.z;
	ME(2,0) = c.x;
	ME(2,1) = c.y;
	ME(2,2) = c.z;
}

// SetColumns
void mat3_t::SetColumns( const vec3_t& a, const vec3_t& b, const vec3_t& c )
{
	ME(0,0) = a.x;
	ME(1,0) = a.y;
	ME(2,0) = a.z;
	ME(0,1) = b.x;
	ME(1,1) = b.y;
	ME(2,1) = b.z;
	ME(0,2) = c.x;
	ME(1,2) = c.y;
	ME(2,2) = c.z;
}

// GetRows
void mat3_t::GetRows( vec3_t& a, vec3_t& b, vec3_t& c ) const
{
	a.x = ME(0,0);
	a.y = ME(0,1);
	a.z = ME(0,2);
	b.x = ME(1,0);
	b.y = ME(1,1);
	b.z = ME(1,2);
	c.x = ME(2,0);
	c.y = ME(2,1);
	c.z = ME(2,2);
}

// GetColumns
void mat3_t::GetColumns( vec3_t& a, vec3_t& b, vec3_t& c ) const
{
	a.x = ME(0,0);
	a.y = ME(1,0);
	a.z = ME(2,0);
	b.x = ME(0,1);
	b.y = ME(1,1);
	b.z = ME(2,1);
	c.x = ME(0,2);
	c.y = ME(1,2);
	c.z = ME(2,2);
}

// Set
// quat
void mat3_t::Set( const quat_t& q )
{
	DEBUG_ERR( !IsZero( 1.0f - q.Length()) ); // Not normalized quat

	float xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;

	xs = q.x+q.x;
	ys = q.y+q.y;
	zs = q.z+q.z;
	wx = q.w*xs;
	wy = q.w*ys;
	wz = q.w*zs;
	xx = q.x*xs;
	xy = q.x*ys;
	xz = q.x*zs;
	yy = q.y*ys;
	yz = q.y*zs;
	zz = q.z*zs;

	ME(0,0) = 1.0f - (yy + zz);
	ME(0,1) = xy - wz;
	ME(0,2) = xz + wy;

	ME(1,0) = xy + wz;
	ME(1,1) = 1.0f - (xx + zz);
	ME(1,2) = yz - wx;

	ME(2,0) = xz - wy;
	ME(2,1) = yz + wx;
	ME(2,2) = 1.0f - (xx + yy);
}

// Set
// euler
void  mat3_t::Set( const euler_t& e )
{
	float ch, sh, ca, sa, cb, sb;
  SinCos( e.heading(), sh, ch );
  SinCos( e.attitude(), sa, ca );
  SinCos( e.bank(), sb, cb );

  ME(0, 0) = ch * ca;
  ME(0, 1) = sh*sb - ch*sa*cb;
  ME(0, 2) = ch*sa*sb + sh*cb;
  ME(1, 0) = sa;
  ME(1, 1) = ca*cb;
  ME(1, 2) = -ca*sb;
  ME(2, 0) = -sh*ca;
  ME(2, 1) = sh*sa*cb + ch*sb;
  ME(2, 2) = -sh*sa*sb + ch*cb;
}

// Set
// axis angles
void mat3_t::Set( const axisang_t& axisang )
{
	DEBUG_ERR( !IsZero( 1.0f-axisang.axis.Length() ) ); // Not normalized axis

	float c, s;
	SinCos( axisang.ang, s, c );
	float t = 1.0f - c;

	const vec3_t& axis = axisang.axis;
	ME(0,0) = c + axis.x*axis.x*t;
	ME(1,1) = c + axis.y*axis.y*t;
	ME(2,2) = c + axis.z*axis.z*t;

	float tmp1 = axis.x*axis.y*t;
	float tmp2 = axis.z*s;
	ME(1,0) = tmp1 + tmp2;
	ME(0,1) = tmp1 - tmp2;
	tmp1 = axis.x*axis.z*t;
	tmp2 = axis.y*s;
	ME(2,0) = tmp1 - tmp2;
	ME(0,2) = tmp1 + tmp2;    tmp1 = axis.y*axis.z*t;
	tmp2 = axis.x*s;
	ME(2,1) = tmp1 + tmp2;
	ME(1,2) = tmp1 - tmp2;

}

// SetRotationX
void mat3_t::SetRotationX( float rad )
{
	float sintheta, costheta;
	SinCos( rad, sintheta, costheta );

	ME(0,0) = 1.0f;
	ME(0,1) = 0.0f;
	ME(0,2) = 0.0f;
	ME(1,0) = 0.0f;
	ME(1,1) = costheta;
	ME(1,2) = -sintheta;
	ME(2,0) = 0.0f;
	ME(2,1) = sintheta;
	ME(2,2) = costheta;
}

// SetRotationY
void mat3_t::SetRotationY( float rad )
{
	float sintheta, costheta;
	SinCos( rad, sintheta, costheta );

	ME(0,0) = costheta;
	ME(0,1) = 0.0f;
	ME(0,2) = sintheta;
	ME(1,0) = 0.0f;
	ME(1,1) = 1.0f;
	ME(1,2) = 0.0f;
	ME(2,0) = -sintheta;
	ME(2,1) = 0.0f;
	ME(2,2) = costheta;
}

// LoadRotationZ
void mat3_t::SetRotationZ( float rad )
{
	float sintheta, costheta;
	SinCos( rad, sintheta, costheta );

	ME(0,0) = costheta;
	ME(0,1) = -sintheta;
	ME(0,2) = 0.0f;
	ME(1,0) = sintheta;
	ME(1,1) = costheta;
	ME(1,2) = 0.0f;
	ME(2,0) = 0.0f;
	ME(2,1) = 0.0f;
	ME(2,2) = 1.0f;
}

// RotateXAxis
/* the slow code is in comments and above the comments the optimized one
If we analize the mat3 we can extract the 3 unit vectors rotated by the mat3. The 3 rotated vectors are in mat's colomns.
This means that: mat3.colomn[0] == i*mat3. RotateXAxis() rotates rad angle not from i vector (aka x axis) but
from the vector from colomn 0*/
void mat3_t::RotateXAxis( float rad )
{
	float sina, cosa;
	SinCos( rad, sina, cosa );

	/*vec3_t x_axis, y_axis, z_axis;
	GetColumns( x_axis, y_axis, z_axis );*/

	// z_axis = z_axis*cosa - y_axis*sina;
	ME(0,2) = ME(0,2)*cosa - ME(0,1)*sina;
	ME(1,2) = ME(1,2)*cosa - ME(1,1)*sina;
	ME(2,2) = ME(2,2)*cosa - ME(2,1)*sina;

	// z_axis.Normalize();
	float len = InvSqrt( ME(0,2)*ME(0,2) + ME(1,2)*ME(1,2) + ME(2,2)*ME(2,2) );
	ME(0,2) *= len;
	ME(1,2) *= len;
	ME(2,2) *= len;

	// y_axis = z_axis * x_axis;
	ME(0,1) = ME(1,2)*ME(2,0) - ME(2,2)*ME(1,0);
	ME(1,1) = ME(2,2)*ME(0,0) - ME(0,2)*ME(2,0);
	ME(2,1) = ME(0,2)*ME(1,0) - ME(1,2)*ME(0,0);

	// y_axis.Normalize();
	/*len = InvSqrt( ME(0,1)*ME(0,1) + ME(1,1)*ME(1,1) + ME(2,1)*ME(2,1) );
	ME(0,1) *= len;
	ME(1,1) *= len;
	ME(2,1) *= len;*/

	// SetColumns( x_axis, y_axis, z_axis );

}

// RotateYAxis
void mat3_t::RotateYAxis( float rad )
{
	float sina, cosa;
	SinCos( rad, sina, cosa );

	/*vec3_t x_axis, y_axis, z_axis;
	GetColumns( x_axis, y_axis, z_axis );*/

	// z_axis = z_axis*cosa + x_axis*sina;
	ME(0,2) = ME(0,2)*cosa + ME(0,0)*sina;
	ME(1,2) = ME(1,2)*cosa + ME(1,0)*sina;
	ME(2,2) = ME(2,2)*cosa + ME(2,0)*sina;

	// z_axis.Normalize();
	float len = InvSqrt( ME(0,2)*ME(0,2) + ME(1,2)*ME(1,2) + ME(2,2)*ME(2,2) );
	ME(0,2) *= len;
	ME(1,2) *= len;
	ME(2,2) *= len;

	// x_axis = (z_axis*y_axis) * -1.0f;
	ME(0,0) = ME(2,2)*ME(1,1) - ME(1,2)*ME(2,1);
	ME(1,0) = ME(0,2)*ME(2,1) - ME(2,2)*ME(0,1);
	ME(2,0) = ME(1,2)*ME(0,1) - ME(0,2)*ME(1,1);

	// x_axis.Normalize();
	/*len = InvSqrt( ME(0,0)*ME(0,0) + ME(1,0)*ME(1,0) + ME(2,0)*ME(2,0) );
	ME(0,0) *= len;
	ME(1,0) *= len;
	ME(2,0) *= len;*/

	// SetColumns( x_axis, y_axis, z_axis );
}


// RotateZAxis
void mat3_t::RotateZAxis( float rad )
{
	float sina, cosa;
	SinCos( rad, sina, cosa );

	/*vec3_t x_axis, y_axis, z_axis;
	GetColumns( x_axis, y_axis, z_axis );*/

	// x_axis = x_axis*cosa + y_axis*sina;
	ME(0,0) = ME(0,0)*cosa + ME(0,1)*sina;
	ME(1,0) = ME(1,0)*cosa + ME(1,1)*sina;
	ME(2,0) = ME(2,0)*cosa + ME(2,1)*sina;

	// x_axis.Normalize();
	float len = InvSqrt( ME(0,0)*ME(0,0) + ME(1,0)*ME(1,0) + ME(2,0)*ME(2,0) );
	ME(0,0) *= len;
	ME(1,0) *= len;
	ME(2,0) *= len;

	// y_axis = z_axis*x_axis;
	ME(0,1) = ME(1,2)*ME(2,0) - ME(2,2)*ME(1,0);
	ME(1,1) = ME(2,2)*ME(0,0) - ME(0,2)*ME(2,0);
	ME(2,1) = ME(0,2)*ME(1,0) - ME(1,2)*ME(0,0);

	// y_axis.Normalize();
	/*len = InvSqrt( ME(0,1)*ME(0,1) + ME(1,1)*ME(1,1) + ME(2,1)*ME(2,1) );
	ME(0,1) *= len;
	ME(1,1) *= len;
	ME(2,1) *= len;*/

	//SetColumns( x_axis, y_axis, z_axis );
}

// Transpose
void mat3_t::Transpose()
{
	float temp = ME(0,1);
	ME(0,1) = ME(1,0);
	ME(1,0) = temp;
	temp = ME(0,2);
	ME(0,2) = ME(2,0);
	ME(2,0) = temp;
	temp = ME(1,2);
	ME(1,2) = ME(2,1);
	ME(2,1) = temp;
}

// Transposed
mat3_t mat3_t::Transposed() const
{
	mat3_t m3;
	m3[0] = ME[0];
	m3[1] = ME[3];
	m3[2] = ME[6];
	m3[3] = ME[1];
	m3[4] = ME[4];
	m3[5] = ME[7];
	m3[6] = ME[2];
	m3[7] = ME[5];
	m3[8] = ME[8];
	return m3;
}

// Reorthogonalize
void mat3_t::Reorthogonalize()
{
	// method 1: standard orthogonalization method
	/*mat3_t correction_m3 =
	(
		(mat3_t::ident * 3.0f) -
		(ME * ME.Transposed())
	) * 0.5f;

	ME = correction_m3 * ME;*/

	// method 2: Gram-Schmidt method with a twist for z_axis
	vec3_t x_axis, y_axis, z_axis;
	GetColumns( x_axis, y_axis, z_axis );

	x_axis.Normalize();

	y_axis = y_axis - ( x_axis * x_axis.Dot(y_axis) );
	y_axis.Normalize();

	z_axis = x_axis.Cross(y_axis);

	SetColumns( x_axis, y_axis, z_axis );
}

// SetIdent
void mat3_t::SetIdent()
{
	ME = ident;
}

// SetZero
void mat3_t::SetZero()
{
	ME[0] = ME[1] = ME[2] = ME[3] = ME[4] = ME[5] = ME[6] = ME[7] = ME[8] = 0.0f;
}

// Print
void mat3_t::Print() const
{
	for( int i=0; i<3; i++ )
	{
		for( int j=0; j<3; j++ )
			cout << fixed << ME(i, j) << " ";
		cout << endl;
	}
	cout << endl;
}

// Determinant
float mat3_t::Det() const
{
	/* accurate method:
	return ME(0, 0)*ME(1, 1)*ME(2, 2) + ME(0, 1)*ME(1, 2)*ME(2, 0) + ME(0, 2)*ME(1, 0)*ME(2, 1)
	- ME(0, 0)*ME(1, 2)*ME(2, 1) - ME(0, 1)*ME(1, 0)*ME(2, 2) - ME(0, 2)*ME(1, 1)*ME(2, 0);*/
	return ME(0, 0)*( ME(1, 1)*ME(2, 2) - ME(1, 2)*ME(2, 1) ) -
	ME(0, 1)*( ME(1, 0)*ME(2, 2) - ME(1, 2)*ME(2, 0) ) +
	ME(0, 2)*( ME(0, 1)*ME(2, 1) - ME(1, 1)*ME(2, 0) );
}


// Invert
// using Gramer's method ( Inv(A) = ( 1/Det(A) ) * Adj(A)  )
mat3_t mat3_t::Inverted() const
{
	mat3_t result;

	// compute determinant
	float cofactor0 = ME(1,1)*ME(2,2) - ME(1,2)*ME(2,1);
	float cofactor3 = ME(0,2)*ME(2,1) - ME(0,1)*ME(2,2);
	float cofactor6 = ME(0,1)*ME(1,2) - ME(0,2)*ME(1,1);
	float det = ME(0,0)*cofactor0 + ME(1,0)*cofactor3 + ME(2,0)*cofactor6;

	DEBUG_ERR( IsZero( det ) ); // Cannot invert det == 0

	// create adjoint matrix and multiply by 1/det to get inverse
	float invDet = 1.0f/det;
	result(0,0) = invDet*cofactor0;
	result(0,1) = invDet*cofactor3;
	result(0,2) = invDet*cofactor6;

	result(1,0) = invDet*(ME(1,2)*ME(2,0) - ME(1,0)*ME(2,2));
	result(1,1) = invDet*(ME(0,0)*ME(2,2) - ME(0,2)*ME(2,0));
	result(1,2) = invDet*(ME(0,2)*ME(1,0) - ME(0,0)*ME(1,2));

	result(2,0) = invDet*(ME(1,0)*ME(2,1) - ME(1,1)*ME(2,0));
	result(2,1) = invDet*(ME(0,1)*ME(2,0) - ME(0,0)*ME(2,1));
	result(2,2) = invDet*(ME(0,0)*ME(1,1) - ME(0,1)*ME(1,0));

	return result;
}

// Invert
// see above
void mat3_t::Invert()
{
	ME = Inverted();
}


/*
=======================================================================================================================================
mat4_t                                                                                                                                =
=======================================================================================================================================
*/
mat4_t mat4_t::ident( 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0 );
mat4_t mat4_t::zero( 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 );

// constructor
mat4_t::mat4_t( float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33 )
{
	ME(0,0) = m00;
	ME(0,1) = m01;
	ME(0,2) = m02;
	ME(0,3) = m03;
	ME(1,0) = m10;
	ME(1,1) = m11;
	ME(1,2) = m12;
	ME(1,3) = m13;
	ME(2,0) = m20;
	ME(2,1) = m21;
	ME(2,2) = m22;
	ME(2,3) = m23;
	ME(3,0) = m30;
	ME(3,1) = m31;
	ME(3,2) = m32;
	ME(3,3) = m33;
}

// copy
mat4_t& mat4_t::operator =( const mat4_t& b )
{
	ME[0] = b[0];
	ME[1] = b[1];
	ME[2] = b[2];
	ME[3] = b[3];
	ME[4] = b[4];
	ME[5] = b[5];
	ME[6] = b[6];
	ME[7] = b[7];
	ME[8] = b[8];
	ME[9] = b[9];
	ME[10] = b[10];
	ME[11] = b[11];
	ME[12] = b[12];
	ME[13] = b[13];
	ME[14] = b[14];
	ME[15] = b[15];
	return ME;
}

// 4x4 + 4x4
mat4_t mat4_t::operator +( const mat4_t& b ) const
{
	mat4_t c;
	for( int i=0; i<16; i++ )
		c[i] = ME[i] + b[i];
	return c;
}

// 4x4 + 4x4 (self)
mat4_t& mat4_t::operator +=( const mat4_t& b )
{
	for( int i=0; i<16; i++ )
		ME[i] += b[i];
	return ME;
}

// 4x4 - 4x4
mat4_t mat4_t::operator -( const mat4_t& b ) const
{
	mat4_t c;
	for( int i=0; i<16; i++ )
		c[i] = ME[i] - b[i];
	return c;
}

// 4x4 - 4x4 (self)
mat4_t& mat4_t::operator -=( const mat4_t& b )
{
	for( int i=0; i<16; i++ )
		ME[i] -= b[i];
	return ME;
}

// 4x4 * 4x4
mat4_t mat4_t::operator *( const mat4_t& b ) const
{
	mat4_t c;
	c(0,0) = ME(0,0)*b(0,0) + ME(0,1)*b(1,0) + ME(0,2)*b(2,0) + ME(0,3)*b(3,0);
	c(0,1) = ME(0,0)*b(0,1) + ME(0,1)*b(1,1) + ME(0,2)*b(2,1) + ME(0,3)*b(3,1);
	c(0,2) = ME(0,0)*b(0,2) + ME(0,1)*b(1,2) + ME(0,2)*b(2,2) + ME(0,3)*b(3,2);
	c(0,3) = ME(0,0)*b(0,3) + ME(0,1)*b(1,3) + ME(0,2)*b(2,3) + ME(0,3)*b(3,3);
	c(1,0) = ME(1,0)*b(0,0) + ME(1,1)*b(1,0) + ME(1,2)*b(2,0) + ME(1,3)*b(3,0);
	c(1,1) = ME(1,0)*b(0,1) + ME(1,1)*b(1,1) + ME(1,2)*b(2,1) + ME(1,3)*b(3,1);
	c(1,2) = ME(1,0)*b(0,2) + ME(1,1)*b(1,2) + ME(1,2)*b(2,2) + ME(1,3)*b(3,2);
	c(1,3) = ME(1,0)*b(0,3) + ME(1,1)*b(1,3) + ME(1,2)*b(2,3) + ME(1,3)*b(3,3);
	c(2,0) = ME(2,0)*b(0,0) + ME(2,1)*b(1,0) + ME(2,2)*b(2,0) + ME(2,3)*b(3,0);
	c(2,1) = ME(2,0)*b(0,1) + ME(2,1)*b(1,1) + ME(2,2)*b(2,1) + ME(2,3)*b(3,1);
	c(2,2) = ME(2,0)*b(0,2) + ME(2,1)*b(1,2) + ME(2,2)*b(2,2) + ME(2,3)*b(3,2);
	c(2,3) = ME(2,0)*b(0,3) + ME(2,1)*b(1,3) + ME(2,2)*b(2,3) + ME(2,3)*b(3,3);
	c(3,0) = ME(3,0)*b(0,0) + ME(3,1)*b(1,0) + ME(3,2)*b(2,0) + ME(3,3)*b(3,0);
	c(3,1) = ME(3,0)*b(0,1) + ME(3,1)*b(1,1) + ME(3,2)*b(2,1) + ME(3,3)*b(3,1);
	c(3,2) = ME(3,0)*b(0,2) + ME(3,1)*b(1,2) + ME(3,2)*b(2,2) + ME(3,3)*b(3,2);
	c(3,3) = ME(3,0)*b(0,3) + ME(3,1)*b(1,3) + ME(3,2)*b(2,3) + ME(3,3)*b(3,3);
	return c;
}

// 4x4 * 4x4 (self)
mat4_t& mat4_t::operator *=( const mat4_t& b )
{
	ME = ME * b;
	return ME;
}

// 4x4 * vec3
vec3_t mat4_t::operator *( const vec3_t& b ) const
{
	return vec3_t(
		ME(0,0)*b.x + ME(0,1)*b.y + ME(0,2)*b.z + ME(0,3),
		ME(1,0)*b.x + ME(1,1)*b.y + ME(1,2)*b.z + ME(1,3),
		ME(2,0)*b.x + ME(2,1)*b.y + ME(2,2)*b.z + ME(2,3)
	);
}

// 4x4 * vec4
vec4_t mat4_t::operator *( const vec4_t& b ) const
{
	return vec4_t(
		ME(0,0)*b.x + ME(0,1)*b.y + ME(0,2)*b.z + ME(0,3)*b.w,
		ME(1,0)*b.x + ME(1,1)*b.y + ME(1,2)*b.z + ME(1,3)*b.w,
		ME(2,0)*b.x + ME(2,1)*b.y + ME(2,2)*b.z + ME(2,3)*b.w,
		ME(3,0)*b.x + ME(3,1)*b.y + ME(3,2)*b.z + ME(3,3)*b.w
	);
}

// 4x4 * scalar
mat4_t mat4_t::operator *( float f ) const
{
	mat4_t c;
	for( int i=0; i<16; i++ )
		c[i] = ME[i] * f;
	return c;
}

// 4x4 * scalar (self)
mat4_t& mat4_t::operator *=( float f )
{
	for( int i=0; i<16; i++ )
		ME[i] *= f;
	return ME;
}

// ==
bool mat4_t::operator ==( const mat4_t& b ) const
{
	for( int i=0; i<16; i++ )
		if( !IsZero( ME[i]-b[i] ) ) return false;
	return true;
}

// !=
bool mat4_t::operator !=( const mat4_t& b ) const
{
	for( int i=0; i<16; i++ )
		if( !IsZero( ME[i]-b[i] ) ) return true;
	return false;
}

// SetRows
void mat4_t::SetRows( const vec4_t& a, const vec4_t& b, const vec4_t& c, const vec4_t& d )
{
	ME(0,0) = a.x;
	ME(0,1) = a.y;
	ME(0,2) = a.z;
	ME(0,3) = a.w;
	ME(1,0) = b.x;
	ME(1,1) = b.y;
	ME(1,2) = b.z;
	ME(1,3) = b.w;
	ME(2,0) = c.x;
	ME(2,1) = c.y;
	ME(2,2) = c.z;
	ME(2,3) = c.w;
	ME(3,0) = d.x;
	ME(3,1) = d.y;
	ME(3,2) = d.z;
	ME(3,3) = d.w;
}

// SetRow
void mat4_t::SetRow( uint i, const vec4_t& v )
{
	DEBUG_ERR( i > 3 );
	ME(i,0) = v.x;
	ME(i,1) = v.y;
	ME(i,2) = v.z;
	ME(i,3) = v.w;
}

// SetColumns
void mat4_t::SetColumns( const vec4_t& a, const vec4_t& b, const vec4_t& c, const vec4_t& d )
{
	ME(0,0) = a.x;
	ME(1,0) = a.y;
	ME(2,0) = a.z;
	ME(3,0) = a.w;
	ME(0,1) = b.x;
	ME(1,1) = b.y;
	ME(2,1) = b.z;
	ME(3,1) = b.w;
	ME(0,2) = c.x;
	ME(1,2) = c.y;
	ME(2,2) = c.z;
	ME(3,2) = c.w;
	ME(0,3) = d.x;
	ME(1,3) = d.y;
	ME(2,3) = d.z;
	ME(3,3) = d.w;
}

// SetColumn
void mat4_t::SetColumn( uint i, const vec4_t& v )
{
	DEBUG_ERR( i > 3 );
	ME(0,i) = v.x;
	ME(1,i) = v.y;
	ME(2,i) = v.z;
	ME(3,i) = v.w;
}

// Transpose
void mat4_t::Transpose()
{
	float tmp = ME(0,1);
	ME(0,1) = ME(1,0);
	ME(1,0) = tmp;
	tmp = ME(0,2);
	ME(0,2) = ME(2,0);
	ME(2,0) = tmp;
	tmp = ME(0,3);
	ME(0,3) = ME(3,0);
	ME(3,0) = tmp;
	tmp = ME(1,2);
	ME(1,2) = ME(2,1);
	ME(2,1) = tmp;
	tmp = ME(1,3);
	ME(1,3) = ME(3,1);
	ME(3,1) = tmp;
	tmp = ME(2,3);
	ME(2,3) = ME(3,2);
	ME(3,2) = tmp;
}

// Transposed
// return the transposed
mat4_t mat4_t::Transposed() const
{
	mat4_t m4;
	m4[0] = ME[0];
	m4[1] = ME[4];
	m4[2] = ME[8];
	m4[3] = ME[12];
	m4[4] = ME[1];
	m4[5] = ME[5];
	m4[6] = ME[9];
	m4[7] = ME[13];
	m4[8] = ME[2];
	m4[9] = ME[6];
	m4[10] = ME[10];
	m4[11] = ME[14];
	m4[12] = ME[3];
	m4[13] = ME[7];
	m4[14] = ME[11];
	m4[15] = ME[15];
	return m4;
}

// SetIdent
void mat4_t::SetIdent()
{
	ME = ident;
}

// SetZero
void mat4_t::SetZero()
{
	ME[15] = ME[14] = ME[13] = ME[12] = ME[11] = ME[10] = ME[9] = ME[8] = ME[7] = ME[6] = ME[5] =
	ME[4] = ME[3] = ME[2] = ME[1] = ME[0] = 0.0f;
}

// Set
// vec3
void mat4_t::Set( const vec3_t& v )
{
	SetIdent();
	ME(0, 3) = v.x;
	ME(1, 3) = v.y;
	ME(2, 3) = v.z;
}

// Set
// vec4
void mat4_t::Set( const vec4_t& v )
{
	ME(0, 0) = 1.0;
	ME(0, 1) = 0.0;
	ME(0, 2) = 0.0;
	ME(0, 3) = v.x;
	ME(1, 0) = 0.0;
	ME(1, 1) = 1.0;
	ME(1, 2) = 0.0;
	ME(1, 3) = v.y;
	ME(2, 0) = 0.0;
	ME(2, 1) = 0.0;
	ME(2, 2) = 1.0;
	ME(2, 3) = v.z;
	ME(3, 0) = 0.0;
	ME(3, 1) = 0.0;
	ME(3, 2) = 0.0;
	ME(3, 3) = v.w;
}

// 3x3 to 4x4
void mat4_t::Set( const mat3_t& m3 )
{
	ME(0,0) = m3(0,0);
	ME(0,1) = m3(0,1);
	ME(0,2) = m3(0,2);
	ME(1,0) = m3(1,0);
	ME(1,1) = m3(1,1);
	ME(1,2) = m3(1,2);
	ME(2,0) = m3(2,0);
	ME(2,1) = m3(2,1);
	ME(2,2) = m3(2,2);
	ME(3, 0) = ME(3, 1) = ME(3, 2) = ME(0, 3) = ME(1, 3) = ME(2, 3) = 0.0f;
	ME(3, 3) = 1.0f;
}

// Set
// from transformation and rotation
void mat4_t::Set( const vec3_t& transl, const mat3_t& rot )
{
	SetRotationPart(rot);
	SetTranslationPart(transl);
	ME(3,0) = ME(3,1) = ME(3,2) = 0.0;
	ME(3,3) = 1.0f;
}

// Set
// from transformation, rotation and scale
void mat4_t::Set( const vec3_t& translate, const mat3_t& rotate, float scale )
{
	if( !IsZero( scale-1.0 ) )
		SetRotationPart( rotate*scale );
	else
		SetRotationPart( rotate );

	SetTranslationPart( translate );

	ME(3,0) = ME(3,1) = ME(3,2) = 0.0;
	ME(3,3) = 1.0f;
}

// SetRotationPart
void mat4_t::SetRotationPart( const mat3_t& m3 )
{
	ME(0,0) = m3(0,0);
	ME(0,1) = m3(0,1);
	ME(0,2) = m3(0,2);
	ME(1,0) = m3(1,0);
	ME(1,1) = m3(1,1);
	ME(1,2) = m3(1,2);
	ME(2,0) = m3(2,0);
	ME(2,1) = m3(2,1);
	ME(2,2) = m3(2,2);
}

// GetRotationPart
mat3_t mat4_t::GetRotationPart() const
{
	mat3_t m3;
	m3(0,0) = ME(0,0);
	m3(0,1) = ME(0,1);
	m3(0,2) = ME(0,2);
	m3(1,0) = ME(1,0);
	m3(1,1) = ME(1,1);
	m3(1,2) = ME(1,2);
	m3(2,0) = ME(2,0);
	m3(2,1) = ME(2,1);
	m3(2,2) = ME(2,2);
	return m3;
}

// SetTranslationPart
void mat4_t::SetTranslationPart( const vec4_t& v )
{
	ME(0, 3) = v.x;
	ME(1, 3) = v.y;
	ME(2, 3) = v.z;
	ME(3, 3) = v.w;
}

// SetTranslationPart
void mat4_t::SetTranslationPart( const vec3_t& v )
{
	ME(0, 3) = v.x;
	ME(1, 3) = v.y;
	ME(2, 3) = v.z;
}

// GetTranslationPart
vec3_t mat4_t::GetTranslationPart() const
{
	return vec3_t( ME(0, 3), ME(1, 3), ME(2, 3) );
}

// Print
void mat4_t::Print() const
{
	cout << fixed;
	for( int i=0; i<4; i++ )
	{
		for( int j=0; j<4; j++ )
		{
			if( ME(i, j) < 0.0 )
				cout << ME(i, j) << " ";
			else
				cout << " " << ME(i, j) << " ";
		}
		cout << endl;
	}
	cout << endl;
}

// Determinant
float mat4_t::Det() const
{
	return
	ME(0, 3)*ME(1, 2)*ME(2, 1)*ME(3, 0) - ME(0, 2)*ME(1, 3)*ME(2, 1)*ME(3, 0) -
	ME(0, 3)*ME(1, 1)*ME(2, 2)*ME(3, 0) + ME(0, 1)*ME(1, 3)*ME(2, 2)*ME(3, 0) +
	ME(0, 2)*ME(1, 1)*ME(2, 3)*ME(3, 0) - ME(0, 1)*ME(1, 2)*ME(2, 3)*ME(3, 0) -
	ME(0, 3)*ME(1, 2)*ME(2, 0)*ME(3, 1) + ME(0, 2)*ME(1, 3)*ME(2, 0)*ME(3, 1) +
	ME(0, 3)*ME(1, 0)*ME(2, 2)*ME(3, 1) - ME(0, 0)*ME(1, 3)*ME(2, 2)*ME(3, 1) -
	ME(0, 2)*ME(1, 0)*ME(2, 3)*ME(3, 1) + ME(0, 0)*ME(1, 2)*ME(2, 3)*ME(3, 1) +
	ME(0, 3)*ME(1, 1)*ME(2, 0)*ME(3, 2) - ME(0, 1)*ME(1, 3)*ME(2, 0)*ME(3, 2) -
	ME(0, 3)*ME(1, 0)*ME(2, 1)*ME(3, 2) + ME(0, 0)*ME(1, 3)*ME(2, 1)*ME(3, 2) +
	ME(0, 1)*ME(1, 0)*ME(2, 3)*ME(3, 2) - ME(0, 0)*ME(1, 1)*ME(2, 3)*ME(3, 2) -
	ME(0, 2)*ME(1, 1)*ME(2, 0)*ME(3, 3) + ME(0, 1)*ME(1, 2)*ME(2, 0)*ME(3, 3) +
	ME(0, 2)*ME(1, 0)*ME(2, 1)*ME(3, 3) - ME(0, 0)*ME(1, 2)*ME(2, 1)*ME(3, 3) -
	ME(0, 1)*ME(1, 0)*ME(2, 2)*ME(3, 3) + ME(0, 0)*ME(1, 1)*ME(2, 2)*ME(3, 3);
}

// Invert
void mat4_t::Invert()
{
	ME = Inverted();
}

// Inverted
mat4_t mat4_t::Inverted() const
{
	float tmp[12];
	float det;

	mat4_t m4;

	tmp[0] = ME(2,2) * ME(3,3);
	tmp[1] = ME(3,2) * ME(2,3);
	tmp[2] = ME(1,2) * ME(3,3);
	tmp[3] = ME(3,2) * ME(1,3);
	tmp[4] = ME(1,2) * ME(2,3);
	tmp[5] = ME(2,2) * ME(1,3);
	tmp[6] = ME(0,2) * ME(3,3);
	tmp[7] = ME(3,2) * ME(0,3);
	tmp[8] = ME(0,2) * ME(2,3);
	tmp[9] = ME(2,2) * ME(0,3);
	tmp[10] = ME(0,2) * ME(1,3);
	tmp[11] = ME(1,2) * ME(0,3);

	m4(0,0) =  tmp[0]*ME(1,1) + tmp[3]*ME(2,1) + tmp[4]*ME(3,1);
	m4(0,0) -= tmp[1]*ME(1,1) + tmp[2]*ME(2,1) + tmp[5]*ME(3,1);
	m4(0,1) =  tmp[1]*ME(0,1) + tmp[6]*ME(2,1) + tmp[9]*ME(3,1);
	m4(0,1) -= tmp[0]*ME(0,1) + tmp[7]*ME(2,1) + tmp[8]*ME(3,1);
	m4(0,2) =  tmp[2]*ME(0,1) + tmp[7]*ME(1,1) + tmp[10]*ME(3,1);
	m4(0,2) -= tmp[3]*ME(0,1) + tmp[6]*ME(1,1) + tmp[11]*ME(3,1);
	m4(0,3) =  tmp[5]*ME(0,1) + tmp[8]*ME(1,1) + tmp[11]*ME(2,1);
	m4(0,3) -= tmp[4]*ME(0,1) + tmp[9]*ME(1,1) + tmp[10]*ME(2,1);
	m4(1,0) =  tmp[1]*ME(1,0) + tmp[2]*ME(2,0) + tmp[5]*ME(3,0);
	m4(1,0) -= tmp[0]*ME(1,0) + tmp[3]*ME(2,0) + tmp[4]*ME(3,0);
	m4(1,1) =  tmp[0]*ME(0,0) + tmp[7]*ME(2,0) + tmp[8]*ME(3,0);
	m4(1,1) -= tmp[1]*ME(0,0) + tmp[6]*ME(2,0) + tmp[9]*ME(3,0);
	m4(1,2) =  tmp[3]*ME(0,0) + tmp[6]*ME(1,0) + tmp[11]*ME(3,0);
	m4(1,2) -= tmp[2]*ME(0,0) + tmp[7]*ME(1,0) + tmp[10]*ME(3,0);
	m4(1,3) =  tmp[4]*ME(0,0) + tmp[9]*ME(1,0) + tmp[10]*ME(2,0);
	m4(1,3) -= tmp[5]*ME(0,0) + tmp[8]*ME(1,0) + tmp[11]*ME(2,0);

	tmp[0] = ME(2,0)*ME(3,1);
	tmp[1] = ME(3,0)*ME(2,1);
	tmp[2] = ME(1,0)*ME(3,1);
	tmp[3] = ME(3,0)*ME(1,1);
	tmp[4] = ME(1,0)*ME(2,1);
	tmp[5] = ME(2,0)*ME(1,1);
	tmp[6] = ME(0,0)*ME(3,1);
	tmp[7] = ME(3,0)*ME(0,1);
	tmp[8] = ME(0,0)*ME(2,1);
	tmp[9] = ME(2,0)*ME(0,1);
	tmp[10] = ME(0,0)*ME(1,1);
	tmp[11] = ME(1,0)*ME(0,1);

	m4(2,0) = tmp[0]*ME(1,3) + tmp[3]*ME(2,3) + tmp[4]*ME(3,3);
	m4(2,0)-= tmp[1]*ME(1,3) + tmp[2]*ME(2,3) + tmp[5]*ME(3,3);
	m4(2,1) = tmp[1]*ME(0,3) + tmp[6]*ME(2,3) + tmp[9]*ME(3,3);
	m4(2,1)-= tmp[0]*ME(0,3) + tmp[7]*ME(2,3) + tmp[8]*ME(3,3);
	m4(2,2) = tmp[2]*ME(0,3) + tmp[7]*ME(1,3) + tmp[10]*ME(3,3);
	m4(2,2)-= tmp[3]*ME(0,3) + tmp[6]*ME(1,3) + tmp[11]*ME(3,3);
	m4(2,3) = tmp[5]*ME(0,3) + tmp[8]*ME(1,3) + tmp[11]*ME(2,3);
	m4(2,3)-= tmp[4]*ME(0,3) + tmp[9]*ME(1,3) + tmp[10]*ME(2,3);
	m4(3,0) = tmp[2]*ME(2,2) + tmp[5]*ME(3,2) + tmp[1]*ME(1,2);
	m4(3,0)-= tmp[4]*ME(3,2) + tmp[0]*ME(1,2) + tmp[3]*ME(2,2);
	m4(3,1) = tmp[8]*ME(3,2) + tmp[0]*ME(0,2) + tmp[7]*ME(2,2);
	m4(3,1)-= tmp[6]*ME(2,2) + tmp[9]*ME(3,2) + tmp[1]*ME(0,2);
	m4(3,2) = tmp[6]*ME(1,2) + tmp[11]*ME(3,2) + tmp[3]*ME(0,2);
	m4(3,2)-= tmp[10]*ME(3,2) + tmp[2]*ME(0,2) + tmp[7]*ME(1,2);
	m4(3,3) = tmp[10]*ME(2,2) + tmp[4]*ME(0,2) + tmp[9]*ME(1,2);
	m4(3,3)-= tmp[8]*ME(1,2) + tmp[11]*ME(2,2) + tmp[5]*ME(0,2);

	det = ME(0,0)*m4(0,0)+ME(1,0)*m4(0,1)+ME(2,0)*m4(0,2)+ME(3,0)*m4(0,3);
	DEBUG_ERR( IsZero( det ) ); // Cannot invert, det == 0
	det = 1/det;
	m4 *= det;
	return m4;
}

// Lerp
mat4_t mat4_t::Lerp( const mat4_t& b, float t ) const
{
	return (ME*(1.0f-t))+(b*t);
}

// CombineTransformations
mat4_t mat4_t::CombineTransformations( const mat4_t& m0, const mat4_t& m1 )
{
	/* the clean code is:
	mat3_t rot = m0.GetRotationPart() * m1.GetRotationPart();  // combine the rotations
	vec3_t tra = (m1.GetTranslationPart()).Transformed( m0.GetTranslationPart(), m0.GetRotationPart(), 1.0 );
	return mat4_t( tra, rot );
	and the optimized: */
	DEBUG_ERR( !IsZero( m0(3,0)+m0(3,1)+m0(3,2)+m0(3,3)-1.0 ) ||
	           !IsZero( m1(3,0)+m1(3,1)+m1(3,2)+m1(3,3)-1.0 ) ); // one of the 2 mat4 doesnt represent transformation

	mat4_t m4;

	m4(0, 0) = m0(0, 0)*m1(0, 0) + m0(0, 1)*m1(1, 0) + m0(0, 2)*m1(2, 0);
	m4(0, 1) = m0(0, 0)*m1(0, 1) + m0(0, 1)*m1(1, 1) + m0(0, 2)*m1(2, 1);
	m4(0, 2) = m0(0, 0)*m1(0, 2) + m0(0, 1)*m1(1, 2) + m0(0, 2)*m1(2, 2);
	m4(1, 0) = m0(1, 0)*m1(0, 0) + m0(1, 1)*m1(1, 0) + m0(1, 2)*m1(2, 0);
	m4(1, 1) = m0(1, 0)*m1(0, 1) + m0(1, 1)*m1(1, 1) + m0(1, 2)*m1(2, 1);
	m4(1, 2) = m0(1, 0)*m1(0, 2) + m0(1, 1)*m1(1, 2) + m0(1, 2)*m1(2, 2);
	m4(2, 0) = m0(2, 0)*m1(0, 0) + m0(2, 1)*m1(1, 0) + m0(2, 2)*m1(2, 0);
	m4(2, 1) = m0(2, 0)*m1(0, 1) + m0(2, 1)*m1(1, 1) + m0(2, 2)*m1(2, 1);
	m4(2, 2) = m0(2, 0)*m1(0, 2) + m0(2, 1)*m1(1, 2) + m0(2, 2)*m1(2, 2);

	m4(0, 3) = m0(0, 0)*m1(0, 3) + m0(0, 1)*m1(1, 3) + m0(0, 2)*m1(2, 3) + m0(0, 3);
	m4(1, 3) = m0(1, 0)*m1(0, 3) + m0(1, 1)*m1(1, 3) + m0(1, 2)*m1(2, 3) + m0(1, 3);
	m4(2, 3) = m0(2, 0)*m1(0, 3) + m0(2, 1)*m1(1, 3) + m0(2, 2)*m1(2, 3) + m0(2, 3);

	m4(3,0) = m4(3,1) = m4(3,2) = 0.0;
	m4(3,3) = 1.0f;

	return m4;
}
