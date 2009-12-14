#include "m_dflt_header.h"


namespace m {


// accessors
M_INLINE float& euler_t::operator []( uint i )
{
	return (&x)[i];
}

M_INLINE float euler_t::operator []( uint i) const
{
	return (&x)[i];
}

M_INLINE float& euler_t::bank()
{
	return x;
}

M_INLINE float euler_t::bank() const
{
	return x;
}

M_INLINE float& euler_t::heading()
{
	return y;
}

M_INLINE float euler_t::heading() const
{
	return y;
}

M_INLINE float& euler_t::attitude()
{
	return z;
}

M_INLINE float euler_t::attitude() const
{
	return z;
}

// constructor []
M_INLINE euler_t::euler_t()
	: x(0.0), y(0.0), z(0.0)
{}

// constructor [float, float, float]
M_INLINE euler_t::euler_t( float x_, float y_, float z_ )
	: x(x_), y(y_), z(z_)
{}

// constructor [euler]
M_INLINE euler_t::euler_t( const euler_t& b )
	: x(b.x), y(b.y), z(b.z)
{}

// constructor [quat]
M_INLINE euler_t::euler_t( const quat_t& q )
{
	float test = q.x*q.y + q.z*q.w;
	if( test > 0.499 )
	{
		heading() = 2.0 * atan2( q.x, q.w );
		attitude() = PI/2.0;
		bank() = 0.0;
		return;
	}
	if( test < -0.499 )
	{
		heading() = -2.0 * atan2( q.x, q.w );
		attitude() = -PI/2.0;
		bank() = 0.0;
		return;
	}

	float sqx = q.x*q.x;
	float sqy = q.y*q.y;
	float sqz = q.z*q.z;
	heading() = atan2( 2.0*q.y*q.w-2.0*q.x*q.z, 1.0-2.0*sqy-2.0*sqz );
	attitude() = asin( 2.0f*test );
	bank() = atan2( 2.0*q.x*q.w-2.0*q.y*q.z, 1.0-2.0*sqx-2.0*sqz );
}

// constructor [mat3]
M_INLINE euler_t::euler_t( const mat3_t& m3 )
{
	float cx, sx;
	float cy, sy;
	float cz, sz;

	sy = m3(0,2);
	cy = Sqrt( 1.0 - sy*sy );
	// normal case
	if ( !IsZero( cy ) )
	{
		float factor = 1.0/cy;
		sx = -m3(1,2) * factor;
		cx = m3(2,2) * factor;
		sz = -m3(0,1) * factor;
		cz = m3(0,0) * factor;
	}
	// x and z axes aligned
	else
	{
		sz = 0.0;
		cz = 1.0;
		sx = m3(2,1);
		cx = m3(1,1);
	}

	z = atan2f( sz, cz );
	y = atan2f( sy, cy );
	x = atan2f( sx, cx );
}


} // end namespace
