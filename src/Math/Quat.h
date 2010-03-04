#ifndef _QUAT_H_
#define _QUAT_H_

#include "Common.h"
#include "MathForwardDecls.h"


namespace M {


class Quat
{
	public:
		// data members
		float x, y, z, w;
		static Quat zero;
		static Quat ident;
		// constructors & distructors
		explicit Quat();
		explicit Quat( float f );
		explicit Quat( float x, float y, float z, float w );
		explicit Quat( const Vec2& v2, float z, float w );
		explicit Quat( const Vec3& v3, float w );
		explicit Quat( const Vec4& v4 );
		         Quat( const Quat& b );
		explicit Quat( const Mat3& m3 );
		explicit Quat( const Euler& eu );
		explicit Quat( const Axisang& axisang );
		// ops with same
		Quat  operator * ( const Quat& b ) const; ///< 16 muls, 12 adds
		Quat& operator *=( const Quat& b );
		// comparision
		bool operator ==( const Quat& b ) const;
		bool operator !=( const Quat& b ) const;
		// other
		void  setFrom2Vec3( const Vec3& v0, const Vec3& v1 ); ///< calculates a quat from v0 to v1
		float getLength() const;
		void  invert();
		void  conjugate();
		Quat  getConjugated() const;
		void  normalize();
		Quat  getNormalized() const;
		void  print() const;
		float dot( const Quat& b ) const;
		Quat  slerp( const Quat& q1, float t ) const; ///< returns slerp( this, q1, t )
		Quat  getRotated( const Quat& b ) const; ///< The same as Quat * Quat
		void  rotate( const Quat& b ); ///< @see getRotated
};


} // end namespace


#include "Quat.inl.h"


#endif
