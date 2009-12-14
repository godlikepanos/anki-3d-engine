#ifndef _VEC3_H_
#define _VEC3_H_

#include "common.h"
#include "forward_decls.h"


namespace m {


class vec3_t
{
	public:
		// data members
		float x, y, z;
		// accessors
		float& operator []( uint i );
		float  operator []( uint i ) const;
		// constructors & distructors
		explicit vec3_t();
		explicit vec3_t( float f );
		explicit vec3_t( float x, float y, float z );
		explicit vec3_t( const vec2_t& v2, float z );
		         vec3_t( const vec3_t& b );
		explicit vec3_t( const vec4_t& v4 );
		explicit vec3_t( const quat_t& q );
		// ops with same type
		vec3_t  operator + ( const vec3_t& b ) const;
		vec3_t& operator +=( const vec3_t& b );
		vec3_t  operator - ( const vec3_t& b ) const;
		vec3_t& operator -=( const vec3_t& b );
		vec3_t  operator * ( const vec3_t& b ) const;
		vec3_t& operator *=( const vec3_t& b );
		vec3_t  operator / ( const vec3_t& b ) const;
		vec3_t& operator /=( const vec3_t& b );
		vec3_t  operator - () const; // return the negative
		// ops with float
		vec3_t  operator + ( float f ) const;
		vec3_t& operator +=( float f );
		vec3_t  operator - ( float f ) const;
		vec3_t& operator -=( float f );
		vec3_t  operator * ( float f ) const;
		vec3_t& operator *=( float f );
		vec3_t  operator / ( float f ) const;
		vec3_t& operator /=( float f );
		// ops with other types
		vec3_t  operator * ( const mat3_t& m3 ) const;
		// comparision
		bool operator ==( const vec3_t& b ) const;
		bool operator !=( const vec3_t& b ) const;
		// other
		float  Dot( const vec3_t& b ) const;
		vec3_t Cross( const vec3_t& b ) const;
		float  Length() const;
		float  LengthSquared() const;
		float  DistanceSquared( const vec3_t& b ) const;
		void   Normalize();
		vec3_t Normalized() const;
		vec3_t Project( const vec3_t& to_this ) const;
		vec3_t Rotated( const quat_t& q ) const; // returns q * this * q.Conjucated() aka returns a rotated this. 21 muls, 12 adds
		void   Rotate( const quat_t& q );
		vec3_t Lerp( const vec3_t& v1, float t ) const; // return Lerp( this, v1, t )
		void   Print() const;
		// transformations. The faster way is by far the mat4 * vec3 or the Transformed( vec3_t, mat3_t )
		vec3_t Transformed( const vec3_t& translate, const mat3_t& rotate, float scale ) const;
		void   Transform( const vec3_t& translate, const mat3_t& rotate, float scale );
		vec3_t Transformed( const vec3_t& translate, const mat3_t& rotate ) const;
		void   Transform( const vec3_t& translate, const mat3_t& rotate );
		vec3_t Transformed( const vec3_t& translate, const quat_t& rotate, float scale ) const;
		void   Transform( const vec3_t& translate, const quat_t& rotate, float scale );
		vec3_t Transformed( const vec3_t& translate, const quat_t& rotate ) const;
		void   Transform( const vec3_t& translate, const quat_t& rotate );
		vec3_t Transformed( const mat4_t& transform ) const;  // 9 muls, 9 adds
		void   Transform( const mat4_t& transform );
};


} // end namespace


#include "vec3.inl.h"


#endif
