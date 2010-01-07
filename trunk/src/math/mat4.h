#ifndef _MAT4_H_
#define _MAT4_H_

#include "common.h"
#include "forward_decls.h"


namespace m {


class mat4_t
{
	private:
		union
		{
			float arr1[16];
			float arr2[4][4];
		};

	public:
		// access to the data
		float& operator ()( const uint i, const uint j );
		const float& operator ()( const uint i, const uint j ) const;
		float& operator []( const uint i);
		const float& operator []( const uint i) const;
		// constructors & distructors
		explicit mat4_t() {}
		explicit mat4_t( float f );
		explicit mat4_t( float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33 );
		explicit mat4_t( const float arr [] );
		         mat4_t( const mat4_t& b );
		explicit mat4_t( const mat3_t& m3 );
		explicit mat4_t( const vec3_t& v );
		explicit mat4_t( const vec4_t& v );
		explicit mat4_t( const vec3_t& transl, const mat3_t& rot );
		explicit mat4_t( const vec3_t& transl, const mat3_t& rot, float scale );
		// ops with same type
		mat4_t  operator + ( const mat4_t& b ) const;
		mat4_t& operator +=( const mat4_t& b );
		mat4_t  operator - ( const mat4_t& b ) const;
		mat4_t& operator -=( const mat4_t& b );
		mat4_t  operator * ( const mat4_t& b ) const;
		mat4_t& operator *=( const mat4_t& b );
		mat4_t  operator / ( const mat4_t& b ) const;
		mat4_t& operator /=( const mat4_t& b );
		// ops with float
		mat4_t  operator + ( float f ) const;
		mat4_t& operator +=( float f );
		mat4_t  operator - ( float f ) const;
		mat4_t& operator -=( float f );
		mat4_t  operator * ( float f ) const;
		mat4_t& operator *=( float f );
		mat4_t  operator / ( float f ) const;
		mat4_t& operator /=( float f );
		// ops with other types
		vec4_t  operator * ( const vec4_t& v4 ) const; // 16 muls, 12 adds
		// comparision
		bool operator ==( const mat4_t& b ) const;
		bool operator !=( const mat4_t& b ) const;
		// other
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
		mat4_t GetTransposed() const;
		void   Print() const;
		float  Det() const;
		void   Invert();
		mat4_t GetInverse() const;
		mat4_t GetInverseTransformation() const;
		mat4_t Lerp( const mat4_t& b, float t ) const;
		static mat4_t CombineTransformations( const mat4_t& m0, const mat4_t& m1 );  // 12 muls, 27 adds. Something like m4 = m0 * m1 ...
		                                                                             // ...but without touching the 4rth row and allot faster
		static const mat4_t& GetIdentity();
		static const mat4_t& GetZero();
};


// other operators
extern mat4_t operator +( float f, const mat4_t& m4 );
extern mat4_t operator -( float f, const mat4_t& m4 );
extern mat4_t operator *( float f, const mat4_t& m4 );
extern mat4_t operator /( float f, const mat4_t& m4 );


} // end namespace


#include "mat4.inl.h"


#endif
