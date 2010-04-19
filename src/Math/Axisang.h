#ifndef _AXISANG_H_
#define _AXISANG_H_

#include "Common.h"
#include "MathForwardDecls.h"


namespace M {


/// Used for rotations
class Axisang
{
	public:
		// data members
		float ang;
		Vec3 axis;
		// constructors & distructors
		explicit Axisang();
		         Axisang( const Axisang& b );
		explicit Axisang( float rad, const Vec3& axis_ );
		explicit Axisang( const Quat& q );
		explicit Axisang( const Mat3& m3 );
};


} // end namespace


#include "Axisang.inl.h"


#endif
