#include "m_dflt_header.h"


#define ME (*this)


namespace m {


// accessors
M_INLINE float& vec3_t::operator []( uint i )
{
	return (&x)[i];
}

M_INLINE float vec3_t::operator []( uint i ) const
{
	return (&x)[i];
}

// constructor []
M_INLINE vec3_t::vec3_t()
	: x(0.0), y(0.0), z(0.0)
{}

// constructor [float, float, float]
M_INLINE vec3_t::vec3_t( float x_, float y_, float z_ )
	: x(x_), y(y_), z(z_)
{}

// constructor [float]
M_INLINE vec3_t::vec3_t( float f )
	: x(f), y(f), z(f)
{}

// constructor [vec3]
M_INLINE vec3_t::vec3_t( const vec3_t& b )
	: x(b.x), y(b.y), z(b.z)
{}

// constructor [vec2, float]
M_INLINE vec3_t::vec3_t( const vec2_t& v2, float z_ )
	: x(v2.x), y(v2.y), z(z_)
{}

// constructor [vec4]
M_INLINE vec3_t::vec3_t( const vec4_t& v4 )
	: x(v4.x), y(v4.y), z(v4.z)
{}

// constructor [quat]
M_INLINE vec3_t::vec3_t( const quat_t& q )
	: x(q.x), y(q.y), z(q.z)
{}

// +
M_INLINE vec3_t vec3_t::operator +( const vec3_t& b ) const
{
	return vec3_t( x+b.x, y+b.y, z+b.z );
}

// +=
M_INLINE vec3_t& vec3_t::operator +=( const vec3_t& b )
{
	x += b.x;
	y += b.y;
	z += b.z;
	return ME;
}

// -
M_INLINE vec3_t vec3_t::operator -( const vec3_t& b ) const
{
	return vec3_t( x-b.x, y-b.y, z-b.z );
}

// -=
M_INLINE vec3_t& vec3_t::operator -=( const vec3_t& b )
{
	x -= b.x;
	y -= b.y;
	z -= b.z;
	return ME;
}

// *
M_INLINE vec3_t vec3_t::operator *( const vec3_t& b ) const
{
	return vec3_t( x*b.x, y*b.y, z*b.z );
}

// *=
M_INLINE vec3_t& vec3_t::operator *=( const vec3_t& b )
{
	x *= b.x;
	y *= b.y;
	z *= b.z;
	return ME;
}

// /
M_INLINE vec3_t vec3_t::operator /( const vec3_t& b ) const
{
	return vec3_t( x/b.x, y/b.y, z/b.z );
}

// /=
M_INLINE vec3_t& vec3_t::operator /=( const vec3_t& b )
{
	x /= b.x;
	y /= b.y;
	z /= b.z;
	return ME;
}

// negative
M_INLINE vec3_t vec3_t::operator -() const
{
	return vec3_t( -x, -y, -z );
}

// ==
M_INLINE bool vec3_t::operator ==( const vec3_t& b ) const
{
	return ( IsZero(x-b.x) && IsZero(y-b.y) && IsZero(z-b.z) ) ? true : false;
}

// !=
M_INLINE bool vec3_t::operator !=( const vec3_t& b ) const
{
	return ( IsZero(x-b.x) && IsZero(y-b.y) && IsZero(z-b.z) ) ? false : true;
}

// vec3 + float
M_INLINE vec3_t vec3_t::operator +( float f ) const
{
	return ME + vec3_t(f);
}

// float + vec3
M_INLINE vec3_t operator +( float f, const vec3_t& v )
{
	return v+f;
}

// vec3 += float
M_INLINE vec3_t& vec3_t::operator +=( float f )
{
	ME += vec3_t(f);
	return ME;
}

// vec3 - float
M_INLINE vec3_t vec3_t::operator -( float f ) const
{
	return ME - vec3_t(f);
}

// float - vec3
M_INLINE vec3_t operator -( float f, const vec3_t& v )
{
	return vec3_t(f-v.x, f-v.y, f-v.z);
}

// vec3 -= float
M_INLINE vec3_t& vec3_t::operator -=( float f )
{
	ME -= vec3_t(f);
	return ME;
}

// vec3 * float
M_INLINE vec3_t vec3_t::operator *( float f ) const
{
	return ME * vec3_t(f);
}

// float * vec3
M_INLINE vec3_t operator *( float f, const vec3_t& v )
{
	return v*f;
}

// vec3 *= float
M_INLINE vec3_t& vec3_t::operator *=( float f )
{
	ME *= vec3_t(f);
	return ME;
}

// vec3 / float
M_INLINE vec3_t vec3_t::operator /( float f ) const
{
	return ME / vec3_t(f);
}

// float / vec3
M_INLINE vec3_t operator /( float f, const vec3_t& v )
{
	return vec3_t(f/v.x, f/v.y, f/v.z);
}

// vec3 /= float
M_INLINE vec3_t& vec3_t::operator /=( float f )
{
	ME /= vec3_t(f);
	return ME;
}

// Dot
M_INLINE float vec3_t::Dot( const vec3_t& b ) const
{
	return x*b.x + y*b.y + z*b.z;
}

// cross prod
M_INLINE vec3_t vec3_t::Cross( const vec3_t& b ) const
{
	return vec3_t( y*b.z-z*b.y, z*b.x-x*b.z, x*b.y-y*b.x );
}

// Length
M_INLINE float vec3_t::Length() const
{
	return Sqrt( x*x + y*y + z*z );
}

// LengthSquared
M_INLINE float vec3_t::LengthSquared() const
{
	return x*x + y*y + z*z;
}

// DistanceSquared
M_INLINE float vec3_t::DistanceSquared( const vec3_t& b ) const
{
	return (ME-b).LengthSquared();
}

// Normalize
M_INLINE void vec3_t::Normalize()
{
	ME *= InvSqrt( x*x + y*y + z*z );
}

// Normalized (return the normalized)
M_INLINE vec3_t vec3_t::Normalized() const
{
	return ME * InvSqrt( x*x + y*y + z*z );
}

// Project
M_INLINE vec3_t vec3_t::Project( const vec3_t& to_this ) const
{
	return to_this * ( ME.Dot(to_this)/(to_this.Dot(to_this)) );
}

// Rotated
M_INLINE vec3_t vec3_t::Rotated( const quat_t& q ) const
{
	DEBUG_ERR( !IsZero(1.0f-q.Length()) ); // Not normalized quat

	/*float vmult = 2.0f*(q.x*x + q.y*y + q.z*z);
	float crossmult = 2.0*q.w;
	float pmult = crossmult*q.w - 1.0;

	return vec3_t( pmult*x + vmult*q.x + crossmult*(q.y*z - q.z*y),
							   pmult*y + vmult*q.y + crossmult*(q.z*x - q.x*z),
	               pmult*z + vmult*q.z + crossmult*(q.x*y - q.y*x) );*/
	vec3_t q_xyz( q );
	return ME + q_xyz.Cross( q_xyz.Cross(ME) + ME*q.w ) * 2.0;
}

// Rotate
M_INLINE void vec3_t::Rotate( const quat_t& q )
{
	ME = Rotated(q);
}

// Print
M_INLINE void vec3_t::Print() const
{
	for( int i=0; i<3; i++ )
		cout << fixed << ME[i] << " ";
	cout << "\n" << endl;
}

// Lerp
M_INLINE vec3_t vec3_t::Lerp( const vec3_t& v1, float t ) const
{
	return (ME*(1.0f-t))+(v1*t);
}

// Transformed [mat3]
M_INLINE vec3_t vec3_t::Transformed( const vec3_t& translate, const mat3_t& rotate, float scale ) const
{
	return (rotate * (ME * scale)) + translate;
}

// Transform [mat3]
M_INLINE void vec3_t::Transform( const vec3_t& translate, const mat3_t& rotate, float scale )
{
	ME = Transformed( translate, rotate, scale );
}

// Transformed [mat3] no scale
M_INLINE vec3_t vec3_t::Transformed( const vec3_t& translate, const mat3_t& rotate ) const
{
	return (rotate * ME) + translate;
}

// Transform [mat3] no scale
M_INLINE void vec3_t::Transform( const vec3_t& translate, const mat3_t& rotate )
{
	ME = Transformed( translate, rotate );
}

// Transformed [quat]
M_INLINE vec3_t vec3_t::Transformed( const vec3_t& translate, const quat_t& rotate, float scale ) const
{
	return (ME * scale).Rotated(rotate) + translate;
}

// Transform [quat3] no scale
M_INLINE void vec3_t::Transform( const vec3_t& translate, const quat_t& rotate, float scale )
{
	ME = Transformed( translate, rotate, scale );
}

// Transformed [mat4]
M_INLINE vec3_t vec3_t::Transformed( const mat4_t& transform ) const
{
	return vec3_t(
		transform(0,0)*x + transform(0,1)*y + transform(0,2)*z + transform(0,3),
		transform(1,0)*x + transform(1,1)*y + transform(1,2)*z + transform(1,3),
		transform(2,0)*x + transform(2,1)*y + transform(2,2)*z + transform(2,3)
	);
}

// Transformed [mat4]
M_INLINE void vec3_t::Transform( const mat4_t& transform )
{
	ME = Transformed( transform );
}


} // end namespace
