#include "m_dflt_header.h"


#define ME (*this)


namespace m {


// accessors
inline float& vec4_t::operator []( uint i )
{
	return (&x)[i];
}

inline float vec4_t::operator []( uint i ) const
{
	return (&x)[i];
}

// constructor []
inline vec4_t::vec4_t()
	: x(0.0), y(0.0), z(0.0), w(0.0)
{}

// constructor [float]
inline vec4_t::vec4_t( float f )
	: x(f), y(f), z(f), w(f)
{}

// constructor [float, float, float, float]
inline vec4_t::vec4_t( float x_, float y_, float z_, float w_ )
	: x(x_), y(y_), z(z_), w(w_)
{}

// constructor [vec2, float, float]
inline vec4_t::vec4_t( const vec2_t& v2, float z_, float w_ )
	: x(v2.x), y(v2.y), z(z_), w(w_)
{}

// constructor [vec3, float]
inline vec4_t::vec4_t( const vec3_t& v3, float w_ )
	: x(v3.x), y(v3.y), z(v3.z), w(w_)
{}

// constructor [vec4]
inline vec4_t::vec4_t( const vec4_t& b )
	: x(b.x), y(b.y), z(b.z), w(b.w)
{}

// constructor [quat]
inline vec4_t::vec4_t( const quat_t& q )
	: x(q.x), y(q.y), z(q.z), w(q.w)
{}

// +
inline vec4_t vec4_t::operator +( const vec4_t& b ) const
{
	return vec4_t( x+b.x, y+b.y, z+b.z, w+b.w );
}

// +=
inline vec4_t& vec4_t::operator +=( const vec4_t& b )
{
	x += b.x;
	y += b.y;
	z += b.z;
	w += b.w;
	return ME;
}

// -
inline vec4_t vec4_t::operator -( const vec4_t& b ) const
{
	return vec4_t( x-b.x, y-b.y, z-b.z, w-b.w );
}

// -=
inline vec4_t& vec4_t::operator -=( const vec4_t& b )
{
	x -= b.x;
	y -= b.y;
	z -= b.z;
	w -= b.w;
	return ME;
}

// *
inline vec4_t vec4_t::operator *( const vec4_t& b ) const
{
	return vec4_t( x*b.x, y*b.y, z*b.z, w*b.w );
}

// *=
inline vec4_t& vec4_t::operator *=( const vec4_t& b )
{
	x *= b.x;
	y *= b.y;
	z *= b.z;
	w *= b.w;
	return ME;
}

// /
inline vec4_t vec4_t::operator /( const vec4_t& b ) const
{
	return vec4_t( x/b.x, y/b.y, z/b.z, w/b.w );
}

// /=
inline vec4_t& vec4_t::operator /=( const vec4_t& b )
{
	x /= b.x;
	y /= b.y;
	z /= b.z;
	w /= b.w;
	return ME;
}

// negative
inline vec4_t vec4_t::operator -() const
{
	return vec4_t( -x, -y, -z, -w );
}

// ==
inline bool vec4_t::operator ==( const vec4_t& b ) const
{
	return ( IsZero(x-b.x) && IsZero(y-b.y) && IsZero(z-b.z) && IsZero(w-b.w) ) ? true : false;
}

// !=
inline bool vec4_t::operator !=( const vec4_t& b ) const
{
	return ( IsZero(x-b.x) && IsZero(y-b.y) && IsZero(z-b.z) && IsZero(w-b.w) ) ? false : true;
}

// vec4 + float
inline vec4_t vec4_t::operator +( float f ) const
{
	return ME + vec4_t(f);
}

// float + vec4
vec4_t operator +( float f, const vec4_t& v4 )
{
	return v4+f;
}

// vec4 += float
inline vec4_t& vec4_t::operator +=( float f )
{
	ME += vec4_t(f);
	return ME;
}

// vec4 - float
inline vec4_t vec4_t::operator -( float f ) const
{
	return ME - vec4_t(f);
}

// float - vec4
vec4_t operator -( float f, const vec4_t& v4 )
{
	return vec4_t( f-v4.x, f-v4.y, f-v4.z, f-v4.w );
}

// vec4 -= float
inline vec4_t& vec4_t::operator -=( float f )
{
	ME -= vec4_t(f);
	return ME;
}

// vec4 * float
inline vec4_t vec4_t::operator *( float f ) const
{
	return ME * vec4_t(f);
}

// float * vec4
vec4_t operator *( float f, const vec4_t& v4 )
{
	return v4*f;
}

// vec4 *= float
inline vec4_t& vec4_t::operator *=( float f )
{
	ME *= vec4_t(f);
	return ME;
}

// vec4 / float
inline vec4_t vec4_t::operator /( float f ) const
{
	return ME / vec4_t(f);
}

// float / vec4
vec4_t operator /( float f, const vec4_t& v4 )
{
	return vec4_t( f/vec4.x, f/vec4.y, f/vec4.z, f/vec4.w );
}

// vec4 /= float
inline vec4_t& vec4_t::operator /=( float f )
{
	ME /= vec4_t(f);
	return ME;
}

// vec4 * mat4
inline vec4_t vec4_t::operator *( const mat4_t& m4 ) const
{
	return vec4_t(
		x*m4(0,0) + y*m4(1,0) + z*m4(2,0) + w*m4(3,0),
		x*m4(0,1) + y*m4(1,1) + z*m4(2,1) + w*m4(3,1),
		x*m4(0,2) + y*m4(1,2) + z*m4(2,2) + w*m4(3,2),
		x*m4(0,3) + y*m4(1,3) + z*m4(2,3) + w*m4(3,3)
	);
}

// Dot
inline float vec4_t::Dot( const vec4_t& b ) const
{
	return x*b.x + y*b.y + z*b.z + w*b.w;
}

// Length
inline float vec4_t::Length() const
{
	return Sqrt( x*x + y*y + z*z + w*w );
}

// Normalized
inline vec4_t vec4_t::GetNormalized() const
{
	return ME * InvSqrt( x*x +y*y + z*z + w*w );
}

// Normalize
inline void vec4_t::Normalize()
{
	ME *= InvSqrt( x*x +y*y + z*z + w*w );
}

// Print
inline void vec4_t::Print() const
{
	for( int i=0; i<4; i++ )
		cout << fixed << ME[i] << " ";
	cout << "\n" << endl;
}


} // end namespace
