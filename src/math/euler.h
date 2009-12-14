#ifndef _EULER_H_
#define _EULER_H_

#include "common.h"
#include "forward_decls.h"


namespace m {


class euler_t
{
	public:
		// data members
		float x, y, z;
		// accessors
		float& operator []( uint i );
		float  operator []( uint i) const;
		float& bank();
		float  bank() const;
		float& heading();
		float  heading() const;
		float& attitude();
		float  attitude() const;
		// constructors & distructors
		explicit euler_t();
		explicit euler_t( float x, float y, float z  );
		         euler_t( const euler_t& b );
		explicit euler_t( const quat_t& q );
		explicit euler_t( const mat3_t& m3 );
		// other
		void Print() const;
};


} // end namespace


#include "euler.inl.h"


#endif
