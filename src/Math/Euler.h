#ifndef _EULER_H_
#define _EULER_H_

#include "Common.h"
#include "MathForwardDecls.h"


namespace M {


/**
 * @brief Usef for rotations. It cannot describe a rotation accurately though
 */
class Euler
{
	public:
		// data members
		float x, y, z;
		// accessors
		float& operator []( uint i );
		float  operator []( uint i ) const;
		float& bank();
		float  bank() const;
		float& heading();
		float  heading() const;
		float& attitude();
		float  attitude() const;
		// constructors & distructors
		explicit Euler();
		explicit Euler( float x, float y, float z  );
		         Euler( const Euler& b );
		explicit Euler( const Quat& q );
		explicit Euler( const Mat3& m3 );
		// other
		void print() const;
};


} // end namespace


#include "Euler.inl.h"


#endif
