#ifndef _MAT3_H_
#define _MAT3_H_

#include "Common.h"
#include "MathForwardDecls.h"


namespace M {


class Mat3
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
		explicit Mat3() {};
		explicit Mat3( float f );
		explicit Mat3( float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22 );
		explicit Mat3( float arr [] );
		         Mat3( const Mat3& b );
		explicit Mat3( const Quat& q ); // 12 muls, 12 adds
		explicit Mat3( const Euler& eu );
		explicit Mat3( const Axisang& axisang );
		// ops with mat3
		Mat3  operator + ( const Mat3& b ) const;
		Mat3& operator +=( const Mat3& b );
		Mat3  operator - ( const Mat3& b ) const;
		Mat3& operator -=( const Mat3& b );
		Mat3  operator * ( const Mat3& b ) const; // 27 muls, 18 adds
		Mat3& operator *=( const Mat3& b );
		Mat3  operator / ( const Mat3& b ) const;
		Mat3& operator /=( const Mat3& b );
		// ops with float
		Mat3  operator + ( float f ) const;
		Mat3& operator +=( float f );
		Mat3  operator - ( float f ) const;
		Mat3& operator -=( float f );
		Mat3  operator * ( float f ) const;
		Mat3& operator *=( float f );
		Mat3  operator / ( float f ) const;
		Mat3& operator /=( float f );
		// ops with others
		Vec3  operator * ( const Vec3& b ) const;  // 9 muls, 6 adds
		// comparision
		bool operator ==( const Mat3& b ) const;
		bool operator !=( const Mat3& b ) const;
		// other
		void  setRows( const Vec3& a, const Vec3& b, const Vec3& c );
		void  setRow( const uint i, const Vec3& v );
		void  getRows( Vec3& a, Vec3& b, Vec3& c ) const;
		Vec3  getRow( const uint i ) const;
		void  setColumns( const Vec3& a, const Vec3& b, const Vec3& c );
		void  setColumn( const uint i, const Vec3& v );
		void  getColumns( Vec3& a, Vec3& b, Vec3& c ) const;
		Vec3  getColumn( const uint i ) const;
		void  setRotationX( float rad );
		void  setRotationY( float rad );
		void  setRotationZ( float rad );
		void  rotateXAxis( float rad ); // it rotates "this" in the axis defined by the rotation AND not the world axis
		void  rotateYAxis( float rad );
		void  rotateZAxis( float rad );
		void  transpose();
		Mat3  getTransposed() const;
		void  reorthogonalize();
		void  print() const;
		float getDet() const;
		void  invert();
		Mat3  getInverse() const;
		static const Mat3& getZero();
		static const Mat3& getIdentity();		
};


// other operators
extern Mat3 operator +( float f, const Mat3& m3 );
extern Mat3 operator -( float f, const Mat3& m3 );
extern Mat3 operator *( float f, const Mat3& m3 );
extern Mat3 operator /( float f, const Mat3& m3 );


} // end namespace


#include "Mat3.inl.h"


#endif
