#ifndef _AXISANG_H_
#define _AXISANG_H_

#include "common.h"
#include "forward_decls.h"


namespace m {


class axisang_t
{
	public:
		// data members
		float ang;
		vec3_t axis;
		// constructors & distructors
		explicit axisang_t();
		         axisang_t( const axisang_t& b );
		explicit axisang_t( float rad, const vec3_t& axis_ );
		explicit axisang_t( const quat_t& q );
		explicit axisang_t( const mat3_t& m3 );
};


} // end namespace


#include "axisang.inl.h"


#endif
