#ifndef _AXISANG_H_
#define _AXISANG_H_

#include "common.h"
#include "forward_decls.h"


namespace m {


class Axisang
{
	public:
		// data members
		float ang;
		vec3_t axis;
		// constructors & distructors
		explicit Axisang();
		         Axisang( const Axisang& b );
		explicit Axisang( float rad, const vec3_t& axis_ );
		explicit Axisang( const quat_t& q );
		explicit Axisang( const mat3_t& m3 );
};


} // end namespace


#include "Axisang.inl.h"


#endif
