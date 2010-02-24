#ifndef _VEC2_H_
#define _VEC2_H_

#include "common.h"
#include "forward_decls.h"


namespace m {


class vec2_t
{
	public:
		// data members
		float x, y;
		static const vec2_t zero;
		static const vec2_t one;
		// accessors
		float& operator []( uint i );
		float  operator []( uint i ) const;
		// constructors & distructors
		explicit vec2_t();
		explicit vec2_t( float f );
		explicit vec2_t( float x, float y );
		         vec2_t( const vec2_t& b );
		explicit vec2_t( const vec3_t& v3 );
		explicit vec2_t( const vec4_t& v4 );
		// ops with same type
		vec2_t  operator + ( const vec2_t& b ) const;
		vec2_t& operator +=( const vec2_t& b );
		vec2_t  operator - ( const vec2_t& b ) const;
		vec2_t& operator -=( const vec2_t& b );
		vec2_t  operator * ( const vec2_t& b ) const;
		vec2_t& operator *=( const vec2_t& b );
		vec2_t  operator / ( const vec2_t& b ) const;
		vec2_t& operator /=( const vec2_t& b );
		vec2_t  operator - () const;
		// ops with float
		vec2_t  operator + ( float f ) const;
		vec2_t& operator +=( float f );
		vec2_t  operator - ( float f ) const;
		vec2_t& operator -=( float f );
		vec2_t  operator * ( float f ) const;
		vec2_t& operator *=( float f );
		vec2_t  operator / ( float f ) const;
		vec2_t& operator /=( float f );
		// comparision
		bool operator ==( const vec2_t& b ) const;
		bool operator !=( const vec2_t& b ) const;
		// other
		static vec2_t GetZero();
		static vec2_t GetOne();
		float  Length() const;
		void   SetZero();
		void   Normalize();
		vec2_t GetNormalized() const;
		float  Dot( const vec2_t& b ) const;
		void   print() const;
};


// other operators
extern vec2_t operator +( float f, const vec2_t& v2 );
extern vec2_t operator -( float f, const vec2_t& v2 );
extern vec2_t operator *( float f, const vec2_t& v2 );
extern vec2_t operator /( float f, const vec2_t& v2 );


} // end namespace


#include "vec2.inl.h"


#endif
