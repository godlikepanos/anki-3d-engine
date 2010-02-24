#ifndef _VEC4_H_
#define _VEC4_H_

#include "common.h"
#include "forward_decls.h"


namespace m {


class vec4_t
{
	public:
		// data members
		float x, y, z, w;
		// accessors
		float& operator []( uint i );
		float  operator []( uint i ) const;
		// constructors & distructors
		explicit vec4_t();
		explicit vec4_t( float f );
		explicit vec4_t( float x, float y, float z, float w );
		explicit vec4_t( const vec2_t& v2, float z, float w );
		explicit vec4_t( const vec3_t& v3, float w );
		         vec4_t( const vec4_t& b );
		explicit vec4_t( const quat_t& q );
		// ops with same
		vec4_t  operator + ( const vec4_t& b ) const;
		vec4_t& operator +=( const vec4_t& b );
		vec4_t  operator - ( const vec4_t& b ) const;
		vec4_t& operator -=( const vec4_t& b );
		vec4_t  operator * ( const vec4_t& b ) const;
		vec4_t& operator *=( const vec4_t& b );
		vec4_t  operator / ( const vec4_t& b ) const;
		vec4_t& operator /=( const vec4_t& b );
		vec4_t  operator - () const;
		// ops with float
		vec4_t  operator + ( float f ) const;
		vec4_t& operator +=( float f );
		vec4_t  operator - ( float f ) const;
		vec4_t& operator -=( float f );
		vec4_t  operator * ( float f ) const;
		vec4_t& operator *=( float f );
		vec4_t  operator / ( float f ) const;
		vec4_t& operator /=( float f );
		// ops with other
		vec4_t  operator * ( const mat4_t& m4 ) const;
		// comparision
		bool operator ==( const vec4_t& b ) const;
		bool operator !=( const vec4_t& b ) const;
		// other
		float  Length() const;
		vec4_t GetNormalized() const;
		void   Normalize();
		void   print() const;
		float  Dot( const vec4_t& b ) const;
};


// other operators
extern vec4_t operator +( float f, const vec4_t& v4 );
extern vec4_t operator -( float f, const vec4_t& v4 );
extern vec4_t operator *( float f, const vec4_t& v4 );
extern vec4_t operator /( float f, const vec4_t& v4 );


} // end namespace


#include "vec4.inl.h"


#endif
