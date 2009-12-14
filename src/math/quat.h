#ifndef _QUAT_H_
#define _QUAT_H_

#include "common.h"
#include "forward_decls.h"


namespace m {


class quat_t
{
	public:
		// data members
		float x, y, z, w;
		static quat_t zero;
		static quat_t ident;
		// constructors & distructors
		explicit quat_t();
		explicit quat_t( float f );
		explicit quat_t( float x, float y, float z, float w );
		explicit quat_t( const vec2_t& v2, float z, float w );
		explicit quat_t( const vec3_t& v3, float w );
		explicit quat_t( const vec4_t& v4 );
		         quat_t( const quat_t& b );
		explicit quat_t( const mat3_t& m3 );
		explicit quat_t( const euler_t& eu );
		explicit quat_t( const axisang_t& axisang );
		// ops with same
		quat_t  operator * ( const quat_t& b ) const;
		quat_t& operator *=( const quat_t& b );
		// comparision
		bool operator ==( const quat_t& b ) const;
		bool operator !=( const quat_t& b ) const;
		// other
		void   CalcFrom2Vec3( const vec3_t& v0, const vec3_t& v1 ); // calculates the quat from v0 to v1
		float  Length() const;
		void   Invert();
		void   Conjugate();
		quat_t Conjugated() const;
		void   Normalize();
		quat_t Normalized() const;
		void   Print() const;
		float  Dot( const quat_t& b ) const;
		quat_t Slerp( const quat_t& q1, float t ) const; // returns Slerp( this, q1, t )
		quat_t Rotated( const quat_t& b ) const;
		void   Rotate( const quat_t& b );
};


} // end namespace


#include "quat.inl.h"


#endif
