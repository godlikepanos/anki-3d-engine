#include "m_dflt_header.h"


#define ME (*this)

namespace m {

// accessors
inline float& vec2_t::operator []( uint i )
{
	return (&x)[i];
}

inline float vec2_t::operator []( uint i ) const
{
	return (&x)[i];
}

// constructor []
inline vec2_t::vec2_t()
	: x(0.0), y(0.0)
{}

// constructor [float, float]
inline vec2_t::vec2_t( float x_, float y_ )
	: x(x_), y(y_)
{}

// constructor [float]
inline vec2_t::vec2_t( float f ): x(f), y(f)
{}

// constructor [vec2]
inline vec2_t::vec2_t( const vec2_t& b ): x(b.x), y(b.y)
{}

// constructor [vec3]
inline vec2_t::vec2_t( const vec3_t& v3 ): x(v3.x), y(v3.y)
{}

// constructor [vec4]
inline vec2_t::vec2_t( const vec4_t& v4 ): x(v4.x), y(v4.y)
{}

// +
inline vec2_t vec2_t::operator +( const vec2_t& b ) const
{
	return vec2_t( x+b.x, y+b.y );
}

// +=
inline vec2_t& vec2_t::operator +=( const vec2_t& b )
{
	x += b.x;
	y += b.y;
	return ME;
}

// -
inline vec2_t vec2_t::operator -( const vec2_t& b ) const
{
	return vec2_t( x-b.x, y-b.y );
}

// -=
inline vec2_t& vec2_t::operator -=( const vec2_t& b )
{
	x -= b.x;
	y -= b.y;
	return ME;
}

// *
inline vec2_t vec2_t::operator *( const vec2_t& b ) const
{
	return vec2_t( x*b.x, y*b.y );
}

// *=
inline vec2_t& vec2_t::operator *=( const vec2_t& b )
{
	x *= b.x;
	y *= b.y;
	return ME;
}

// /
inline vec2_t vec2_t::operator /( const vec2_t& b ) const
{
	return vec2_t( x/b.x, y/b.y );
}

// /=
inline vec2_t& vec2_t::operator /=( const vec2_t& b )
{
	x /= b.x;
	y /= b.y;
	return ME;
}

// negative
inline vec2_t vec2_t::operator -() const
{
	return vec2_t( -x, -y );
}

// ==
inline bool vec2_t::operator ==( const vec2_t& b ) const
{
	return ( IsZero(x-b.x) && IsZero(y-b.y) ) ? true : false;
}

// !=
inline bool vec2_t::operator !=( const vec2_t& b ) const
{
	return ( IsZero(x-b.x) && IsZero(y-b.y) ) ? false : true;
}

// vec2 + float
inline vec2_t vec2_t::operator +( float f ) const
{
	return ME + vec2_t(f);
}

// float + vec2
inline vec2_t operator +( float f, const vec2_t& v2 )
{
	return v2+f;
}

// vec2 += float
inline vec2_t& vec2_t::operator +=( float f )
{
	ME += vec2_t(f);
	return ME;
}

// vec2 - float
inline vec2_t vec2_t::operator -( float f ) const
{
	return ME - vec2_t(f);
}

// float - vec2
inline vec2_t operator -( float f, const vec2_t& v2 )
{
	return vec2_t( f-v2.x, f-v2.y );
}

// vec2 -= float
inline vec2_t& vec2_t::operator -=( float f )
{
	ME -= vec2_t(f);
	return ME;
}

// vec2 * float
inline vec2_t vec2_t::operator *( float f ) const
{
	return ME * vec2_t(f);
}

// float * vec2
inline vec2_t operator *( float f, const vec2_t& v2 )
{
	return v2*f;
}

// vec2 *= float
inline vec2_t& vec2_t::operator *=( float f )
{
	ME *= vec2_t(f);
	return ME;
}

// vec2 / float
inline vec2_t vec2_t::operator /( float f ) const
{
	return ME / vec2_t(f);
}

// float / vec2
inline vec2_t operator /( float f, const vec2_t& v2 )
{
	return vec2_t( f/v2.x, f/v2.y );
}

// vec2 /= float
inline vec2_t& vec2_t::operator /=( float f )
{
	ME /= vec2_t(f);
	return ME;
}

// Length
inline float vec2_t::Length() const
{
	return Sqrt( x*x + y*y );
}

// set to zero
inline void vec2_t::SetZero()
{
	x = y = 0.0;
}

// Normalize
inline void vec2_t::Normalize()
{
	ME *= InvSqrt( x*x + y*y );
}

// Normalized (return the normalized)
inline vec2_t vec2_t::GetNormalized() const
{
	return ME * InvSqrt( x*x + y*y );
}

// Dot
inline float vec2_t::Dot( const vec2_t& b ) const
{
	return x*b.x + y*b.y;
}

// GetZero
inline vec2_t vec2_t::GetZero()
{
	return vec2_t(0.0);
}

// GetOne
inline vec2_t vec2_t::GetOne()
{
	return vec2_t(1.0);
}

// Print
inline void vec2_t::Print() const
{
	for( int i=0; i<2; i++ )
		cout << fixed << ME[i] << ' ';
	cout << "\n" << endl;
}


} // end namespace
