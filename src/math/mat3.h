#ifndef _MAT3_H_
#define _MAT3_H_

#include "common.h"
#include "forward_decls.h"


namespace m {


class mat3_t
{
	private:
		// data members
		union
		{
			float arr1[9];
			float arr2[3][3];
		};

	public:
		// accessors
		float& operator ()( const uint i, const uint j );
		const float& operator ()( const uint i, const uint j ) const;
		float& operator []( const uint i);
		const float& operator []( const uint i) const;
		// constructors & distructors
		explicit mat3_t() {};
		explicit mat3_t( float f );
		explicit mat3_t( float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22 );
		explicit mat3_t( float arr [] );
		         mat3_t( const mat3_t& b );
		explicit mat3_t( const quat_t& q ); // 12 muls, 12 adds
		explicit mat3_t( const Euler& eu );
		explicit mat3_t( const Axisang& axisang );
		// ops with mat3
		mat3_t  operator + ( const mat3_t& b ) const;
		mat3_t& operator +=( const mat3_t& b );
		mat3_t  operator - ( const mat3_t& b ) const;
		mat3_t& operator -=( const mat3_t& b );
		mat3_t  operator * ( const mat3_t& b ) const; // 27 muls, 18 adds
		mat3_t& operator *=( const mat3_t& b );
		mat3_t  operator / ( const mat3_t& b ) const;
		mat3_t& operator /=( const mat3_t& b );
		// ops with float
		mat3_t  operator + ( float f ) const;
		mat3_t& operator +=( float f );
		mat3_t  operator - ( float f ) const;
		mat3_t& operator -=( float f );
		mat3_t  operator * ( float f ) const;
		mat3_t& operator *=( float f );
		mat3_t  operator / ( float f ) const;
		mat3_t& operator /=( float f );
		// ops with others
		vec3_t  operator * ( const vec3_t& b ) const;  // 9 muls, 6 adds
		// comparision
		bool operator ==( const mat3_t& b ) const;
		bool operator !=( const mat3_t& b ) const;
		// other
		void   SetRows( const vec3_t& a, const vec3_t& b, const vec3_t& c );
		void   SetRow( const uint i, const vec3_t& v );
		void   GetRows( vec3_t& a, vec3_t& b, vec3_t& c ) const;
		vec3_t GetRow( const uint i ) const;
		void   SetColumns( const vec3_t& a, const vec3_t& b, const vec3_t& c );
		void   SetColumn( const uint i, const vec3_t& v );
		void   GetColumns( vec3_t& a, vec3_t& b, vec3_t& c ) const;
		vec3_t GetColumn( const uint i ) const;
		void   SetRotationX( float rad );
		void   SetRotationY( float rad );
		void   SetRotationZ( float rad );
		void   RotateXAxis( float rad ); // it rotates "this" in the axis defined by the rotation AND not the world axis
		void   RotateYAxis( float rad );
		void   RotateZAxis( float rad );
		void   Transpose();
		mat3_t GetTransposed() const;
		void   Reorthogonalize();
		void   print() const;
		float  Det() const;
		void   Invert();
		mat3_t GetInverse() const;
		static const mat3_t& GetZero();
		static const mat3_t& GetIdentity();		
};


// other operators
extern mat3_t operator +( float f, const mat3_t& m3 );
extern mat3_t operator -( float f, const mat3_t& m3 );
extern mat3_t operator *( float f, const mat3_t& m3 );
extern mat3_t operator /( float f, const mat3_t& m3 );


} // end namespace


#include "mat3.inl.h"


#endif
