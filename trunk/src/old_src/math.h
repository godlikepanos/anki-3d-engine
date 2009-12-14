#ifndef _MATH_H_
#define _MATH_H_

#include <stdio.h>
#include <math.h>
#include "common.h"

#define PI 3.14159265358979323846f
#define EPSILON 1.0e-6f


class vec2_t;
class vec4_t;
class vec3_t;
class quat_t;
class euler_t;
class axisang_t;
class mat3_t;
class mat4_t;



/*
=======================================================================================================================================
vector 2                                                                                                                              =
=======================================================================================================================================
*/
class vec2_t
{
	public:
		// data members
		float x, y;
		static vec2_t zero;
		// access to the data
		inline float& operator []( uint i )  { return (&x)[i]; }
		inline float  operator []( uint i ) const { return (&x)[i]; }
		// constructors & distructors
		inline vec2_t(): x(0.0), y(0.0) {}
		inline vec2_t( float x_, float y_ ): x(x_), y(y_) {}
		inline vec2_t( float f ): x(f), y(f) {}
		inline vec2_t( const vec2_t& b ) { (*this) = b; }
		       vec2_t( const vec3_t& v3 );
		       vec2_t( const vec4_t& v4 );
		// ops with same type
		vec2_t& operator = ( const vec2_t& b );
		vec2_t  operator + ( const vec2_t& b ) const;
		vec2_t& operator +=( const vec2_t& b );
		vec2_t  operator - ( const vec2_t& b ) const;
		vec2_t& operator -=( const vec2_t& b );
		vec2_t  operator * ( const vec2_t& b ) const;
		vec2_t& operator *=( const vec2_t& b );
		vec2_t  operator / ( const vec2_t& b ) const;
		vec2_t& operator /=( const vec2_t& b );
		vec2_t  operator - () const; // return the negative
		// other types
		vec2_t  operator * ( float f ) const;
		vec2_t& operator *=( float f );
		vec2_t  operator / ( float f ) const;
		vec2_t& operator /=( float f );
		// comparision
		bool operator ==( const vec2_t& b ) const;
		bool operator !=( const vec2_t& b ) const;
		// other
		float  Length() const;
		void   SetZero();
		void   Normalize();
		vec2_t Normalized() const;
		float  Dot( const vec2_t& b ) const;
		void   Print() const;
};


/*
=======================================================================================================================================
vector 3                                                                                                                              =
used as column matrix in mat*vec                                                                                                      =
=======================================================================================================================================
*/
class vec3_t
{
	public:
		// data members
		float x, y, z;
		static vec3_t zero;
		static vec3_t one;
		static vec3_t i, j, k; // unit vectors
		// access to the data
		inline float& operator []( uint i )  { return (&x)[i]; }
		inline float  operator []( uint i ) const { return (&x)[i]; }
		// constructors & distructors
		inline vec3_t(): x(0.0), y(0.0), z(0.0) {}
		inline vec3_t( float x_, float y_, float z_ ): x(x_), y(y_), z(z_) {}
		inline vec3_t( float f ): x(f), y(f), z(f) {}
		inline vec3_t( const vec3_t& b ) { (*this)=b; }
		inline vec3_t( const vec2_t& v2, float z_ ): x(v2.x), y(v2.y), z(z_) {}
		       vec3_t( const vec4_t& v4 );
		       vec3_t( const quat_t& q );
		// ops with same type
		vec3_t& operator = ( const vec3_t& b );
		vec3_t  operator + ( const vec3_t& b ) const;
		vec3_t& operator +=( const vec3_t& b );
		vec3_t  operator - ( const vec3_t& b ) const;
		vec3_t& operator -=( const vec3_t& b );
		vec3_t  operator * ( const vec3_t& b ) const;
		vec3_t& operator *=( const vec3_t& b );
		vec3_t  operator / ( const vec3_t& b ) const;
		vec3_t& operator /=( const vec3_t& b );
		vec3_t  operator - () const; // return the negative
		// other types
		vec3_t  operator * ( float f ) const;
		vec3_t& operator *=( float f );
		vec3_t  operator / ( float f ) const;
		vec3_t& operator /=( float f );
		vec3_t  operator * ( const mat3_t& m3 ) const;
		// comparision
		bool operator ==( const vec3_t& b ) const;
		bool operator !=( const vec3_t& b ) const;
		// other
		float  Dot( const vec3_t& b ) const;
		vec3_t Cross( const vec3_t& b ) const;
		float  Length() const;
		float  LengthSquared() const;
		float  DistanceSquared( const vec3_t& b ) const { return ((*this)-b).LengthSquared(); }
		void   Normalize();
		vec3_t Normalized() const;
		void   SetZero() { z=y=x=0.0; };
		vec3_t Project( const vec3_t& to_this ) const;
		vec3_t Rotated( const quat_t& q ) const; // returns q * this * q.Conjucated() aka rotates this. 21 muls, 12 adds
		void   Rotate( const quat_t& q );
		vec3_t Lerp( const vec3_t& v1, float t ) const; // return Lerp( this, v1, t )
		void   Print() const;
		// transformations. The faster way is by far the mat4 * vec3 or the Transformed( vec3_t, mat3_t )
		vec3_t Transformed( const vec3_t& translate, const mat3_t& rotate, float scale ) const;
		void   Transform( const vec3_t& translate, const mat3_t& rotate, float scale ) { (*this)=Transformed( translate, rotate, scale ); }
		vec3_t Transformed( const vec3_t& translate, const mat3_t& rotate ) const;
		void   Transform( const vec3_t& translate, const mat3_t& rotate ) { (*this)=Transformed( translate, rotate ); }
		vec3_t Transformed( const vec3_t& translate, const quat_t& rotate, float scale ) const;
		void   Transform( const vec3_t& translate, const quat_t& rotate, float scale ) { (*this)=Transformed( translate, rotate, scale ); }
};


/*
=======================================================================================================================================
vector 4                                                                                                                              =
=======================================================================================================================================
*/
class vec4_t
{
	public:
		// data members
		float x, y, z, w;
		static vec4_t zero, one, i, j, k, l;
		// access to the data
		inline float& operator []( uint i )  { return (&x)[i]; }
		inline float operator []( uint i ) const { return (&x)[i]; }
		// constructors & distructors
		inline vec4_t(): x(0.0), y(0.0), z(0.0), w(0.0) {}
		inline vec4_t( float x_, float y_, float z_, float w_ ): x(x_), y(y_), z(z_), w(w_) {}
		inline vec4_t( float f ): x(f), y(f), z(f), w(f) {}
		inline vec4_t( const vec4_t& b ) { (*this)=b; }
		inline vec4_t( const vec4_t& v2, float z_, float w_ ): x(v2.x), y(v2.y), z(z_), w(w_) {}
		inline vec4_t( const vec3_t& v3, float w_ ): x(v3.x), y(v3.y), z(v3.z), w(w_) {}
		// ops with same
		vec4_t& operator = ( const vec4_t& b );
		vec4_t  operator + ( const vec4_t& b ) const;
		vec4_t& operator +=( const vec4_t& b );
		vec4_t  operator - ( const vec4_t& b ) const;
		vec4_t& operator -=( const vec4_t& b );
		vec4_t  operator - () const; // return the negative
		vec4_t  operator * ( const vec4_t& b ) const; // for color operations
		vec4_t& operator *=( const vec4_t& b );       //       >>
		vec4_t  operator / ( const vec4_t& b ) const;
		vec4_t& operator /=( const vec4_t& b );
		// ops with other
		vec4_t  operator * ( float f ) const;
		vec4_t& operator *=( float f );
		vec4_t  operator / ( float f ) const;
		vec4_t& operator /=( float f );
		vec4_t  operator * ( const mat4_t& m4 ) const;
		// comparision
		bool operator ==( const vec4_t& b ) const;
		bool operator !=( const vec4_t& b ) const;
		// other
		void  SetZero();
		float Length() const;
		void  Normalize();
		void  Print() const;
		float Dot( const vec4_t& b ) const;
};


/*
=======================================================================================================================================
quaternion                                                                                                                            =
=======================================================================================================================================
*/
class quat_t
{
	public:
		// data members
		float w, x, y, z;
		static quat_t zero;
		static quat_t ident;
		// access to the data
		inline float& operator []( uint i )  { return (&w)[i]; }
		inline float operator []( uint i) const { return (&w)[i]; }
		// constructors & distructors
		inline quat_t() {}
		inline quat_t( float w_, float x_, float y_, float z_ ): w(w_), x(x_), y(y_), z(z_) {}
		inline quat_t( const quat_t& q ) { (*this)=q; }
		inline quat_t( const mat3_t& m3 ) { Set(m3); }
		inline quat_t( const euler_t& eu ) { Set(eu); }
		inline quat_t( const axisang_t& axisang ) { Set( axisang ); }
		inline quat_t( const vec3_t& v0, const vec3_t& v1 ) { CalcFromVecVec( v0, v1 ); }
		inline quat_t( const vec3_t& v ) { Set(v); }
		inline quat_t( const vec4_t& v ) { Set(v); }
		// ops with same
		quat_t& operator = ( const quat_t& b );
		quat_t  operator + ( const quat_t& b ) const;
		quat_t& operator +=( const quat_t& b );
		quat_t  operator - ( const quat_t& b ) const;
		quat_t& operator -=( const quat_t& b );
		quat_t  operator * ( const quat_t& b ) const;
		quat_t& operator *=( const quat_t& b );
		// ops with other
		quat_t  operator * ( float f ) const; // used to combine rotations. 16 muls and 12 adds
		quat_t& operator *=( float f );
		quat_t  operator / ( float f ) const;
		quat_t& operator /=( float f );
		// comparision
		bool operator ==( const quat_t& b ) const;
		bool operator !=( const quat_t& b ) const;
		// other
		void   Set( const mat3_t& m );
		void   Set( const euler_t& e );
		void   Set( const axisang_t& axisang );
		void   CalcFromVecVec( const vec3_t& v0, const vec3_t& v1 ); // calculates the quat from v0 to v1
		void   Set( const vec3_t& v ); // quat is: v.x*i + v.y*j + v.z*k + 0.0
		void   Set( const vec4_t& v ); // quat is: v.x*i + v.y*j + v.z*k + v.w
		void   SetIdent();
		void   SetZero();
		float  Length() const;
		void   Invert();
		void   Conjugate();
		quat_t Conjugated() const;
		void   Normalize();
		void   Print() const;
		float  Dot( const quat_t& b ) const;
		quat_t Slerp( const quat_t& q1, float t ) const; // returns Slerp( this, q1, t )
};


/*
=======================================================================================================================================
euler angles in RAD                                                                                                                   =
=======================================================================================================================================
*/
class euler_t
{
	public:
		// data members
		float x, y, z;
		// access to the data
		inline float& operator []( uint i )      { return (&x)[i]; }
		inline float  operator []( uint i) const { return (&x)[i]; }
		float& bank()           { return x; }
		float  bank() const     { return x; }
		float& heading()        { return y; }
		float  heading() const  { return y; }
		float& attitude()       { return z; }
		float  attitude() const { return z; }
		// constructors & distructors
		inline euler_t() {}
		inline euler_t( float x_, float y_, float z_  ) { x=x_; y=y_; z=z_; }
		inline euler_t( const euler_t& b ) { (*this)=b; }
		inline euler_t( const quat_t& q ) { Set(q); }
		inline euler_t( const mat3_t& m3 ) { Set(m3); }
		// ops with same
		euler_t& operator = ( const euler_t& b );
		// other
		void SetZero();
		void Set( const quat_t& q );
		void Set( const mat3_t& m3 );
		void Print() const;
};


/*
=======================================================================================================================================
axis orientation                                                                                                                      =
=======================================================================================================================================
*/
class axisang_t
{
	public:
		// data members
		float ang;
		vec3_t axis;
		// constructors & distructors
		inline axisang_t() {}
		inline axisang_t( float rad, const vec3_t& axis_ ) { ang=rad; axis=axis_; }
		inline axisang_t( const quat_t& q ) { Set(q); }
		inline axisang_t( const mat3_t& m3 ) { Set(m3); }
		// misc
		void Set( const quat_t& q );
		void Set( const mat3_t& m3 );
};


/*
=======================================================================================================================================
matrix 3x3                                                                                                                            =
=======================================================================================================================================
*/
class mat3_t
{
	public:
		// data members
		float data_m3[9];
		static mat3_t ident;
		static mat3_t zero;
		// accessors
		inline float& operator ()( uint i, uint j ) { return data_m3[3*i+j]; }
		inline float  operator ()( uint i, uint j ) const { return data_m3[3*i+j]; }
		inline float& operator []( uint i) { return data_m3[i]; }    // used to access the array serial. It must be data_m3[i] cause its optimized
		inline float  operator []( uint i) const { return data_m3[i]; }
		// constructors & distructors
		inline mat3_t() {}
		       mat3_t( float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22 );
		inline mat3_t( const mat3_t& b ) { (*this)=b; }
		inline mat3_t( const quat_t& q ) { Set(q); }
		inline mat3_t( const euler_t& eu ) { Set(eu); }
		inline mat3_t( const axisang_t& axisang ) { Set(axisang); }
		// ops with mat3
		mat3_t& operator = ( const mat3_t& b );
		mat3_t  operator + ( const mat3_t& b ) const;
		mat3_t& operator +=( const mat3_t& b );
		mat3_t  operator - ( const mat3_t& b ) const;
		mat3_t& operator -=( const mat3_t& b );
		mat3_t  operator * ( const mat3_t& b ) const; // 27 muls, 18 adds
		mat3_t& operator *=( const mat3_t& b );
		// ops with others
		vec3_t  operator * ( const vec3_t& b ) const;  // 9 muls, 6 adds
		mat3_t  operator * ( float f ) const;
		mat3_t& operator *=( float f );
		// comparision
		bool operator ==( const mat3_t& b ) const;
		bool operator !=( const mat3_t& b ) const;
		// other
		void   SetRows( const vec3_t& a, const vec3_t& b, const vec3_t& c );
		void   SetRow( const uint i, const vec3_t& v ) { (*this)(i,0)=v.x; (*this)(i,1)=v.y; (*this)(i,2)=v.z; }
		void   GetRows( vec3_t& a, vec3_t& b, vec3_t& c ) const;
		vec3_t GetRow( const uint i ) const { return vec3_t( (*this)(i,0), (*this)(i,1), (*this)(i,2) ); }
		void   SetColumns( const vec3_t& a, const vec3_t& b, const vec3_t& c );
		void   SetColumn( const uint i, const vec3_t& v ) { (*this)(0,i)=v.x; (*this)(1,i)=v.y; (*this)(2,i)=v.z; }
		void   GetColumns( vec3_t& a, vec3_t& b, vec3_t& c ) const;
		vec3_t GetColumn( const uint i ) const { return vec3_t( (*this)(0,i), (*this)(1,i), (*this)(2,i) ); }
		void   Set( const quat_t& q ); // 12 muls, 12 adds
		void   Set( const euler_t& e );
		void   Set( const axisang_t& axisang );
		void   SetRotationX( float rad );
		void   SetRotationY( float rad );
		void   SetRotationZ( float rad );
		void   RotateXAxis( float rad ); // it rotates "this" in the axis defined by the rotation AND not the world axis
		void   RotateYAxis( float rad );
		void   RotateZAxis( float rad );
		void   Transpose();
		mat3_t Transposed() const;
		void   Reorthogonalize();
		void   SetIdent();
		void   SetZero();
		void   Print() const;
		float  Det() const;
		void   Invert();
		mat3_t Inverted() const;
};


/*
=======================================================================================================================================
matrix 4x4                                                                                                                            =
=======================================================================================================================================
*/
class mat4_t
{
	public:
		// data members
		float data_m4[16];
		static mat4_t ident;
		static mat4_t zero;
		// access to the data
		inline float& operator ()( uint i, uint j )  { return data_m4[4*i+j]; }
		inline float  operator ()( uint i, uint j ) const { return data_m4[4*i+j]; }
		inline float& operator []( uint i) { return data_m4[i]; }
		inline float  operator []( uint i) const { return data_m4[i]; }
		// constructors & distructors
		inline mat4_t()  {}
		       mat4_t( float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33 );
		inline mat4_t( const mat4_t& m4 )  { (*this)=m4; }
		inline mat4_t( const mat3_t& m3 )  { Set(m3); }
		inline mat4_t( const vec3_t& v ) { Set(v); }
		inline mat4_t( const vec4_t& v ) { Set(v); }
		inline mat4_t( const vec3_t& transl, const mat3_t& rot ) { Set(transl, rot); }
		inline mat4_t( const vec3_t& transl, const mat3_t& rot, float scale ) { Set(transl, rot, scale); }
		// with same
		mat4_t& operator = ( const mat4_t& b );
		mat4_t  operator + ( const mat4_t& b ) const;
		mat4_t& operator +=( const mat4_t& b );
		mat4_t  operator - ( const mat4_t& b ) const;
		mat4_t& operator -=( const mat4_t& b );
		mat4_t  operator * ( const mat4_t& b ) const;  // 64 muls, 48 adds. Use CombineTransformations instead
		mat4_t& operator *=( const mat4_t& b );        //      >>
		// with other types
		vec3_t  operator * ( const vec3_t& v3 ) const; // 9 muls, 9 adds
		vec4_t  operator * ( const vec4_t& v4 ) const; // 16 muls, 12 adds
		mat4_t  operator * ( float f ) const;
		mat4_t& operator *=( float f );
		// comparision
		bool operator ==( const mat4_t& b ) const;
		bool operator !=( const mat4_t& b ) const;
		// other
		void   Set( const mat3_t& m3 ); // sets the rotation part equal to mat3 and the rest like the ident
		void   Set( const vec3_t& v ); // sets the translation part equal to vec3 and the rest like the ident
		void   Set( const vec4_t& v ); // sets the translation part equal to vec4 and the rest like the ident
		void   Set( const vec3_t& transl, const mat3_t& rot ); // this = mat4_t(transl) * mat4_t(rot)
		void   Set( const vec3_t& transl, const mat3_t& rot, float scale ); // this = mat4_t(transl) * mat4_t(rot) * mat4_t(scale). 9 muls
		void   SetIdent();
		void   SetZero();
		void   SetRows( const vec4_t& a, const vec4_t& b, const vec4_t& c, const vec4_t& d );
		void   SetRow( uint i, const vec4_t& v );
		void   SetColumns( const vec4_t& a, const vec4_t& b, const vec4_t& c, const vec4_t& d );
		void   SetColumn( uint i, const vec4_t& v );
		void   SetRotationPart( const mat3_t& m3 );
		void   SetTranslationPart( const vec4_t& v4 );
		mat3_t GetRotationPart() const;
		void   SetTranslationPart( const vec3_t& v3 );
		vec3_t GetTranslationPart() const;
		void   Transpose();
		mat4_t Transposed() const;
		void   Print() const;
		float  Det() const;
		void   Invert();
		mat4_t Inverted() const;
		mat4_t Lerp( const mat4_t& b, float t ) const;
		static mat4_t CombineTransformations( const mat4_t& m0, const mat4_t& m1 );  // 12 muls, 27 adds. Something like m4 = m0 * m1 ...
		                                                                             // ...but without touching the 4rth row and allot faster
};



/*
=======================================================================================================================================
misc                                                                                                                                  =
=======================================================================================================================================
*/
extern void  MathSanityChecks();
extern void  SinCos( float rad, float& sin_, float& cos_ );
extern float InvSqrt( float f );
inline float Sqrt( float f ) { return 1/InvSqrt(f); }
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
	tf = t1.Transformed( t0, r0, s0 );
	rf = r0 * r1;
	sf = s0 * s1;
}

//  CombineTransformations as the above but without scale
inline void CombineTransformations( const vec3_t& t0, const mat3_t& r0, const vec3_t& t1, const mat3_t& r1, vec3_t& tf, mat3_t& rf)
{
	tf = t1.Transformed( t0, r0 );
	rf = r0 * r1;
}


#endif
