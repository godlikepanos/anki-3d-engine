#include "m_dflt_header.h"


#define ME (*this)

namespace m {


// constructor []
inline quat_t::quat_t()
	: x(0.0), y(0.0), z(0.0), w(1.0)
{}

// constructor [float]
inline quat_t::quat_t( float f )
	: x(f), y(f), z(f), w(f)
{}

// constructor [float, float, float, float]
inline quat_t::quat_t( float x_, float y_, float z_, float w_ )
	: x(x_), y(y_), z(z_), w(w_)
{}

// constructor [vec2, float, float]
inline quat_t::quat_t( const vec2_t& v2, float z_, float w_ )
	: x(v2.x), y(v2.y), z(z_), w(w_)
{}

// constructor [vec3, float]
inline quat_t::quat_t( const vec3_t& v3, float w_ )
	: x(v3.x), y(v3.y), z(v3.z), w(w_)
{}

// constructor [vec4]
inline quat_t::quat_t( const vec4_t& v4 )
	: x(v4.x), y(v4.y), z(v4.z), w(v4.w)
{}

// constructor [quat]
inline quat_t::quat_t( const quat_t& b )
	: x(b.x), y(b.y), z(b.z), w(b.w)
{}

// constructor [mat3]
inline quat_t::quat_t( const mat3_t& m3 )
{
	float trace = m3(0, 0) + m3(1, 1) + m3(2, 2) + 1.0;
	if( trace > EPSILON )
	{
		float s = 0.5 * InvSqrt(trace);
		w = 0.25 / s;
		x = ( m3(2, 1) - m3(1, 2) ) * s;
		y = ( m3(0, 2) - m3(2, 0) ) * s;
		z = ( m3(1, 0) - m3(0, 1) ) * s;
	}
	else
	{
		if( m3(0, 0) > m3(1, 1) && m3(0, 0) > m3(2, 2) )
		{
			float s = 0.5 * InvSqrt( 1.0 + m3(0, 0) - m3(1, 1) - m3(2, 2) );
			w = (m3(1, 2) - m3(2, 1) ) * s;
			x = 0.25 / s;
			y = (m3(0, 1) + m3(1, 0) ) * s;
			z = (m3(0, 2) + m3(2, 0) ) * s;
		}
		else if( m3(1, 1) > m3(2, 2) )
		{
			float s = 0.5 * InvSqrt( 1.0 + m3(1, 1) - m3(0, 0) - m3(2, 2) );
			w = (m3(0, 2) - m3(2, 0) ) * s;
			x = (m3(0, 1) + m3(1, 0) ) * s;
			y = 0.25 / s;
			z = (m3(1, 2) + m3(2, 1) ) * s;
		}
		else
		{
			float s = 0.5 * InvSqrt( 1.0 + m3(2, 2) - m3(0, 0) - m3(1, 1) );
			w = (m3(0, 1) - m3(1, 0) ) * s;
			x = (m3(0, 2) + m3(2, 0) ) * s;
			y = (m3(1, 2) + m3(2, 1) ) * s;
			z = 0.25 / s;
		}
	}
}

// constructor [euler]
inline quat_t::quat_t( const euler_t& eu )
{
	float cx, sx;
	SinCos( eu.heading()*0.5, sx, cx );

	float cy, sy;
	SinCos( eu.attitude()*0.5, sy, cy );

	float cz, sz;
	SinCos( eu.bank()*0.5, sz, cz );

	float cxcy = cx*cy;
	float sxsy = sx*sy;
	x = cxcy*sz + sxsy*cz;
	y = sx*cy*cz + cx*sy*sz;
	z = cx*sy*cz - sx*cy*sz;
	w = cxcy*cz - sxsy*sz;
}

// constructor [euler]
inline quat_t::quat_t( const axisang_t& axisang )
{
	float lengthsq = axisang.axis.LengthSquared();
	if( IsZero( lengthsq ) )
	{
		ME = ident;
		return;
	}

	float rad = axisang.ang * 0.5;

	float sintheta, costheta;
	SinCos( rad, sintheta, costheta );

	float scalefactor = sintheta * InvSqrt(lengthsq);

	x = scalefactor * axisang.axis.x;
	y = scalefactor * axisang.axis.y;
	z = scalefactor * axisang.axis.z;
	w = costheta;
}

// *
inline quat_t quat_t::operator *( const quat_t& b ) const
{
	return quat_t(
		 x * b.w + y * b.z - z * b.y + w * b.x,
		-x * b.z + y * b.w + z * b.x + w * b.y,
		 x * b.y - y * b.x + z * b.w + w * b.z,
		-x * b.x - y * b.y - z * b.z + w * b.w
	);
}

// *=
inline quat_t& quat_t::operator *=( const quat_t& b )
{
	ME = ME * b;
	return ME;
}

// ==
inline bool quat_t::operator ==( const quat_t& b ) const
{
	return ( IsZero(x-b.x) && IsZero(y-b.y) && IsZero(z-b.z) && IsZero(w-b.w) ) ? true : false;
}

// !=
inline bool quat_t::operator !=( const quat_t& b ) const
{
	return ( IsZero(x-b.x) && IsZero(y-b.y) && IsZero(z-b.z) && IsZero(w-b.w) ) ? false : true;
}

// Conjugate
inline void quat_t::Conjugate()
{
	x = -x;
	y = -y;
	z = -z;
}

// Conjugated
inline quat_t quat_t::Conjugated() const
{
	return quat_t( -x, -y, -z, w );
}

// Normalized
inline quat_t quat_t::GetNormalized() const
{
	return quat_t( vec4_t(ME).GetNormalized() );
}

// Normalize
inline void quat_t::Normalize()
{
	ME = GetNormalized();
}

// Length
inline float quat_t::Length() const
{
	return Sqrt( w*w + x*x + y*y + z*z );
}

// Invert
inline void quat_t::Invert()
{
	float norm = w*w + x*x + y*y + z*z;

	DEBUG_ERR( IsZero(norm) ); // Norm is zero

	float normi = 1.0 / norm;
	ME = quat_t( -normi*x, -normi*y, -normi*z, normi*w );
}

// Print
inline void quat_t::Print() const
{
	cout << fixed << "(w,x,y,z) = " << w << ' ' << x << ' ' << y << ' ' << z  << '\n' << endl;
}

// CalcFromVecVec
inline void quat_t::CalcFrom2Vec3( const vec3_t& from, const vec3_t& to )
{
	vec3_t axis( from.Cross(to) );
	ME = quat_t( axis.x, axis.y, axis.z, from.Dot(to) );
	Normalize();
	w += 1.0;

	if( w <= EPSILON )
	{
		if( from.z*from.z > from.x*from.x )
			ME = quat_t( 0.0, from.z, -from.y, 0.0 );
		else
			ME = quat_t( from.y, -from.x, 0.0, 0.0 );
	}
	Normalize();
}

// Rotated
inline quat_t quat_t::GetRotated( const quat_t& b ) const
{
	return ME * b;
}

// Rotate
inline void quat_t::Rotate( const quat_t& b )
{
	ME = GetRotated( b );
}

// Dot
inline float quat_t::Dot( const quat_t& b ) const
{
	return w*b.w + x*b.x + y*b.y + z*b.z;
}

// SLERP
inline quat_t quat_t::Slerp( const quat_t& q1_, float t ) const
{
	const quat_t& q0 = ME;
	quat_t q1( q1_ );
	float cos_half_theta = q0.w*q1.w + q0.x*q1.x + q0.y*q1.y + q0.z*q1.z;
	if( cos_half_theta < 0.0 )
	{
		q1 = quat_t( -vec4_t(q1) ); // quat changes
		cos_half_theta = -cos_half_theta;
	}
	if( fabs(cos_half_theta) >= 1.0f )
	{
		return quat_t( q0 );
	}

	float half_theta = acos( cos_half_theta );
	float sin_half_theta = Sqrt(1.0 - cos_half_theta*cos_half_theta);

	if( fabs(sin_half_theta) < 0.001 )
	{
		return quat_t( (vec4_t(q0) + vec4_t(q1))*0.5 );
	}
	float ratio_a = sin((1.0 - t) * half_theta) / sin_half_theta;
	float ratio_b = sin(t * half_theta) / sin_half_theta;
	vec4_t tmp, tmp1, sum;
	tmp = vec4_t(q0)*ratio_a;
	tmp1 = vec4_t(q1)*ratio_b;
	sum = tmp + tmp1;
	sum.Normalize();
	return quat_t( sum );
}


} // end namespace
