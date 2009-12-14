#include "m_dflt_header.h"


#define ME (*this)

namespace m {

// accessors
M_INLINE float& vec2_t::operator []( uint i )
{
	return (&x)[i];
}

M_INLINE float vec2_t::operator []( uint i ) const
{
	return (&x)[i];
}

// constructor []
M_INLINE vec2_t::vec2_t()
	: x(0.0), y(0.0)
{}

// constructor [float, float]
M_INLINE vec2_t::vec2_t( float x_, float y_ )
	: x(x_), y(y_)
{}

// constructor [float]
M_INLINE vec2_t::vec2_t( float f ): x(f), y(f)
{}

// constructor [vec2]
M_INLINE vec2_t::vec2_t( const vec2_t& b ): x(b.x), y(b.y)
{}

// constructor [vec3]
M_INLINE vec2_t::vec2_t( const vec3_t& v3 ): x(v3.x), y(v3.y)
{}

// constructor [vec4]
M_INLINE vec2_t::vec2_t( const vec4_t& v4 ): x(v4.x), y(v4.y)
{}

// +
M_INLINE vec2_t vec2_t::operator +( const vec2_t& b ) const
{
	return vec2_t( x+b.x, y+b.y );
}

// +=
M_INLINE vec2_t& vec2_t::operator +=( const vec2_t& b )
{
	x += b.x;
	y += b.y;
	return ME;
}

// -
M_INLINE vec2_t vec2_t::operator -( const vec2_t& b ) const
{
	return vec2_t( x-b.x, y-b.y );
}

// -=
M_INLINE vec2_t& vec2_t::operator -=( const vec2_t& b )
{
	x -= b.x;
	y -= b.y;
	return ME;
}

// *
M_INLINE vec2_t vec2_t::operator *( const vec2_t& b ) const
{
	return vec2_t( x*b.x, y*b.y );
}

// *=
M_INLINE vec2_t& vec2_t::operator *=( const vec2_t& b )
{
	x *= b.x;
	y *= b.y;
	return ME;
}

// /
M_INLINE vec2_t vec2_t::operator /( const vec2_t& b ) const
{
	return vec2_t( x/b.x, y/b.y );
}

// /=
M_INLINE vec2_t& vec2_t::operator /=( const vec2_t& b )
{
	x /= b.x;
	y /= b.y;
	return ME;
}

// negative
M_INLINE vec2_t vec2_t::operator -() const
{
	return vec2_t( -x, -y );
}

// ==
M_INLINE bool vec2_t::operator ==( const vec2_t& b ) const
{
	return ( IsZero(x-b.x) && IsZero(y-b.y) ) ? true : false;
}

// !=
M_INLINE bool vec2_t::operator !=( const vec2_t& b ) const
{
	return ( IsZero(x-b.x) && IsZero(y-b.y) ) ? false : true;
}

// vec2 + float
M_INLINE vec2_t vec2_t::operator +( float f ) const
{
	return ME + vec2_t(f);
}

// vec2 += float
M_INLINE vec2_t& vec2_t::operator +=( float f )
{
	ME += vec2_t(f);
	return ME;
}

// vec2 - float
M_INLINE vec2_t vec2_t::operator -( float f ) const
{
	return ME - vec2_t(f);
}

// vec2 -= float
M_INLINE vec2_t& vec2_t::operator -=( float f )
{
	ME -= vec2_t(f);
	return ME;
}

// vec2 * float
M_INLINE vec2_t vec2_t::operator *( float f ) const
{
	return ME * vec2_t(f);
}

// vec2 *= float
M_INLINE vec2_t& vec2_t::operator *=( float f )
{
	ME *= vec2_t(f);
	return ME;
}

// vec2 / float
M_INLINE vec2_t vec2_t::operator /( float f ) const
{
	return ME / vec2_t(f);
}

// vec2 /= float
M_INLINE vec2_t& vec2_t::operator /=( float f )
{
	ME /= vec2_t(f);
	return ME;
}

// Length
M_INLINE float vec2_t::Length() const
{
	return Sqrt( x*x + y*y );
}

// set to zero
M_INLINE void vec2_t::SetZero()
{
	x = y = 0.0;
}

// Normalize
M_INLINE void vec2_t::Normalize()
{
	ME *= InvSqrt( x*x + y*y );
}

// Normalized (return the normalized)
M_INLINE vec2_t vec2_t::Normalized() const
{
	return ME * InvSqrt( x*x + y*y );
}

// Dot
M_INLINE float vec2_t::Dot( const vec2_t& b ) const
{
	return x*b.x + y*b.y;
}

// GetZero
M_INLINE vec2_t vec2_t::GetZero()
{
	return vec2_t(0.0);
}

// GetOne
M_INLINE vec2_t vec2_t::GetOne()
{
	return vec2_t(1.0);
}

// Print
M_INLINE void vec2_t::Print() const
{
	for( int i=0; i<2; i++ )
		cout << fixed << ME[i] << ' ';
	cout << "\n" << endl;
}


} // end namespace
