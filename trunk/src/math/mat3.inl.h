#include "m_dflt_header.h"


#define ME (*this)


namespace m {

// accessors
inline float& mat3_t::operator ()( const uint i, const uint j )
{
	return arr2[i][j];
}

inline const float& mat3_t::operator ()( const uint i, const uint j ) const
{
	return arr2[i][j]; 
}

inline float& mat3_t::operator []( const uint i)
{
	return arr1[i];
}

inline const float& mat3_t::operator []( const uint i) const
{
	return arr1[i];
}

// constructor [float[]]
inline mat3_t::mat3_t( float arr [] )
{
	for( int i=0; i<9; i++ )
		ME[i] = arr[i];
}

// constructor [float...........float]
inline mat3_t::mat3_t( float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22 )
{
	ME(0,0) = m00;
	ME(0,1) = m01;
	ME(0,2) = m02;
	ME(1,0) = m10;
	ME(1,1) = m11;
	ME(1,2) = m12;
	ME(2,0) = m20;
	ME(2,1) = m21;
	ME(2,2) = m22;
}

// constructor [mat3]
inline mat3_t::mat3_t( const mat3_t& b )
{
	for( int i=0; i<9; i++ )
		ME[i] = b[i];
}

// constructor [quat]
inline mat3_t::mat3_t( const quat_t& q )
{
	DEBUG_ERR( !IsZero( 1.0f - q.Length()) ); // Not normalized quat

	float xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;

	xs = q.x+q.x;
	ys = q.y+q.y;
	zs = q.z+q.z;
	wx = q.w*xs;
	wy = q.w*ys;
	wz = q.w*zs;
	xx = q.x*xs;
	xy = q.x*ys;
	xz = q.x*zs;
	yy = q.y*ys;
	yz = q.y*zs;
	zz = q.z*zs;

	ME(0,0) = 1.0 - (yy + zz);
	ME(0,1) = xy - wz;
	ME(0,2) = xz + wy;

	ME(1,0) = xy + wz;
	ME(1,1) = 1.0f - (xx + zz);
	ME(1,2) = yz - wx;

	ME(2,0) = xz - wy;
	ME(2,1) = yz + wx;
	ME(2,2) = 1.0 - (xx + yy);
}

// constructor [euler]
inline mat3_t::mat3_t( const euler_t& e )
{
	float ch, sh, ca, sa, cb, sb;
  SinCos( e.heading(), sh, ch );
  SinCos( e.attitude(), sa, ca );
  SinCos( e.bank(), sb, cb );

  ME(0,0) = ch * ca;
  ME(0,1) = sh*sb - ch*sa*cb;
  ME(0,2) = ch*sa*sb + sh*cb;
  ME(1,0) = sa;
  ME(1,1) = ca*cb;
  ME(1,2) = -ca*sb;
  ME(2,0) = -sh*ca;
  ME(2,1) = sh*sa*cb + ch*sb;
  ME(2,2) = -sh*sa*sb + ch*cb;
}

// constructor [axisang]
inline mat3_t::mat3_t( const axisang_t& axisang )
{
	DEBUG_ERR( !IsZero( 1.0-axisang.axis.Length() ) ); // Not normalized axis

	float c, s;
	SinCos( axisang.ang, s, c );
	float t = 1.0 - c;

	const vec3_t& axis = axisang.axis;
	ME(0,0) = c + axis.x*axis.x*t;
	ME(1,1) = c + axis.y*axis.y*t;
	ME(2,2) = c + axis.z*axis.z*t;

	float tmp1 = axis.x*axis.y*t;
	float tmp2 = axis.z*s;
	ME(1,0) = tmp1 + tmp2;
	ME(0,1) = tmp1 - tmp2;
	tmp1 = axis.x*axis.z*t;
	tmp2 = axis.y*s;
	ME(2,0) = tmp1 - tmp2;
	ME(0,2) = tmp1 + tmp2;    tmp1 = axis.y*axis.z*t;
	tmp2 = axis.x*s;
	ME(2,1) = tmp1 + tmp2;
	ME(1,2) = tmp1 - tmp2;
}

// constructor [float]
inline mat3_t::mat3_t( float f )
{
	for( int i=0; i<9; i++ )
			ME[i] = f;
}

// 3x3 + 3x3
inline mat3_t mat3_t::operator +( const mat3_t& b ) const
{
	mat3_t c;
	for( int i=0; i<9; i++ )
		c[i] = ME[i] + b[i];
	return c;
}

// 3x3 += 3x3
inline mat3_t& mat3_t::operator +=( const mat3_t& b )
{
	for( int i=0; i<9; i++ )
		ME[i] += b[i];
	return ME;
}

// 3x3 - 3x3
inline mat3_t mat3_t::operator -( const mat3_t& b ) const
{
	mat3_t c;
	for( int i=0; i<9; i++ )
		c[i] = ME[i] - b[i];
	return c;
}

// 3x3 -= 3x3
inline mat3_t& mat3_t::operator -=( const mat3_t& b )
{
	for( int i=0; i<9; i++ )
		ME[i] -= b[i];
	return ME;
}

// 3x3 * 3x3
inline mat3_t mat3_t::operator *( const mat3_t& b ) const
{
	mat3_t c;
	c(0, 0) = ME(0, 0)*b(0, 0) + ME(0, 1)*b(1, 0) + ME(0, 2)*b(2, 0);
	c(0, 1) = ME(0, 0)*b(0, 1) + ME(0, 1)*b(1, 1) + ME(0, 2)*b(2, 1);
	c(0, 2) = ME(0, 0)*b(0, 2) + ME(0, 1)*b(1, 2) + ME(0, 2)*b(2, 2);
	c(1, 0) = ME(1, 0)*b(0, 0) + ME(1, 1)*b(1, 0) + ME(1, 2)*b(2, 0);
	c(1, 1) = ME(1, 0)*b(0, 1) + ME(1, 1)*b(1, 1) + ME(1, 2)*b(2, 1);
	c(1, 2) = ME(1, 0)*b(0, 2) + ME(1, 1)*b(1, 2) + ME(1, 2)*b(2, 2);
	c(2, 0) = ME(2, 0)*b(0, 0) + ME(2, 1)*b(1, 0) + ME(2, 2)*b(2, 0);
	c(2, 1) = ME(2, 0)*b(0, 1) + ME(2, 1)*b(1, 1) + ME(2, 2)*b(2, 1);
	c(2, 2) = ME(2, 0)*b(0, 2) + ME(2, 1)*b(1, 2) + ME(2, 2)*b(2, 2);
	return c;
}

// 3x3 *= 3x3
inline mat3_t& mat3_t::operator *=( const mat3_t& b )
{
	ME = ME * b;
	return ME;
}

// 3x3 + float
inline mat3_t mat3_t::operator +( float f ) const
{
	mat3_t c;
	for( uint i=0; i<9; i++ )
		c[i] = ME[i] + f;
	return c;
}

// float + 3x3
inline mat3_t operator +( float f, const mat3_t& m3 )
{
	return m3+f;
}

// 3x3 += float
inline mat3_t& mat3_t::operator +=( float f )
{
	for( uint i=0; i<9; i++ )
		ME[i] += f;
	return ME;
}

// 3x3 - float
inline mat3_t mat3_t::operator -( float f ) const
{
	mat3_t c;
	for( uint i=0; i<9; i++ )
		c[i] = ME[i] - f;
	return c;
}

// float - 3x3
inline mat3_t operator -( float f, const mat3_t& m3 )
{
	mat3_t out;
	for( uint i=0; i<9; i++ )
		out[i] = f - m3[i];
	return out;
}

// 3x3 -= float
inline mat3_t& mat3_t::operator -=( float f )
{
	for( uint i=0; i<9; i++ )
		ME[i] -= f;
	return ME;
}

// 3x3 * float
inline mat3_t mat3_t::operator *( float f ) const
{
	mat3_t c;
	for( uint i=0; i<9; i++ )
		c[i] = ME[i] * f;
	return c;
}

// float * 3x3
inline mat3_t operator *( float f, const mat3_t& m3 )
{
	mat3_t out;
	for( uint i=0; i<9; i++ )
		out[i] = f * m3[i];
	return out;
}

// 3x3 *= float
inline mat3_t& mat3_t::operator *=( float f )
{
	for( uint i=0; i<9; i++ )
		ME[i] *= f;
	return ME;
}

// 3x3 / float
inline mat3_t mat3_t::operator /( float f ) const
{
	mat3_t c;
	for( uint i=0; i<9; i++ )
		c[i] = ME[i] / f;
	return c;
}

// float / 3x3
inline mat3_t operator /( float f, const mat3_t& m3 )
{
	mat3_t out;
	for( uint i=0; i<9; i++ )
		out[i] = f / m3[i];
	return out;
}

// 3x3 / float (self)
inline mat3_t& mat3_t::operator /=( float f )
{
	for( uint i=0; i<9; i++ )
		ME[i] /= f;
	return ME;
}

// 3x3 * vec3 (cross products with rows)
inline vec3_t mat3_t::operator *( const vec3_t& b ) const
{
	return vec3_t(
		ME(0, 0)*b.x + ME(0, 1)*b.y + ME(0, 2)*b.z,
		ME(1, 0)*b.x + ME(1, 1)*b.y + ME(1, 2)*b.z,
		ME(2, 0)*b.x + ME(2, 1)*b.y + ME(2, 2)*b.z
	);
}

// ==
inline bool mat3_t::operator ==( const mat3_t& b ) const
{
	for( int i=0; i<9; i++ )
		if( !IsZero( ME[i]-b[i] ) ) return false;
	return true;
}

// !=
inline bool mat3_t::operator !=( const mat3_t& b ) const
{
	for( int i=0; i<9; i++ )
		if( !IsZero( ME[i]-b[i] ) ) return true;
	return false;
}

// SetRows
inline void mat3_t::SetRows( const vec3_t& a, const vec3_t& b, const vec3_t& c )
{
	ME(0,0) = a.x;
	ME(0,1) = a.y;
	ME(0,2) = a.z;
	ME(1,0) = b.x;
	ME(1,1) = b.y;
	ME(1,2) = b.z;
	ME(2,0) = c.x;
	ME(2,1) = c.y;
	ME(2,2) = c.z;
}

// SetColumns
inline void mat3_t::SetColumns( const vec3_t& a, const vec3_t& b, const vec3_t& c )
{
	ME(0,0) = a.x;
	ME(1,0) = a.y;
	ME(2,0) = a.z;
	ME(0,1) = b.x;
	ME(1,1) = b.y;
	ME(2,1) = b.z;
	ME(0,2) = c.x;
	ME(1,2) = c.y;
	ME(2,2) = c.z;
}

// GetRows
inline void mat3_t::GetRows( vec3_t& a, vec3_t& b, vec3_t& c ) const
{
	a.x = ME(0,0);
	a.y = ME(0,1);
	a.z = ME(0,2);
	b.x = ME(1,0);
	b.y = ME(1,1);
	b.z = ME(1,2);
	c.x = ME(2,0);
	c.y = ME(2,1);
	c.z = ME(2,2);
}

// GetColumns
inline void mat3_t::GetColumns( vec3_t& a, vec3_t& b, vec3_t& c ) const
{
	a.x = ME(0,0);
	a.y = ME(1,0);
	a.z = ME(2,0);
	b.x = ME(0,1);
	b.y = ME(1,1);
	b.z = ME(2,1);
	c.x = ME(0,2);
	c.y = ME(1,2);
	c.z = ME(2,2);
}

// SetRow
inline void mat3_t::SetRow( const uint i, const vec3_t& v )
{
	ME(i,0)=v.x;
	ME(i,1)=v.y;
	ME(i,2)=v.z;
}

// GetRow
inline vec3_t mat3_t::GetRow( const uint i ) const
{
	return vec3_t( ME(i,0), ME(i,1), ME(i,2) );
}

// SetColumn
inline void mat3_t::SetColumn( const uint i, const vec3_t& v )
{
	ME(0,i)=v.x;
	ME(1,i)=v.y;
	ME(2,i)=v.z;
}

// GetColumn
inline vec3_t mat3_t::GetColumn( const uint i ) const
{
	return vec3_t( ME(0,i), ME(1,i), ME(2,i) );
}

// SetRotationX
inline void mat3_t::SetRotationX( float rad )
{
	float sintheta, costheta;
	SinCos( rad, sintheta, costheta );

	ME(0,0) = 1.0f;
	ME(0,1) = 0.0f;
	ME(0,2) = 0.0f;
	ME(1,0) = 0.0f;
	ME(1,1) = costheta;
	ME(1,2) = -sintheta;
	ME(2,0) = 0.0f;
	ME(2,1) = sintheta;
	ME(2,2) = costheta;
}

// SetRotationY
inline void mat3_t::SetRotationY( float rad )
{
	float sintheta, costheta;
	SinCos( rad, sintheta, costheta );

	ME(0,0) = costheta;
	ME(0,1) = 0.0f;
	ME(0,2) = sintheta;
	ME(1,0) = 0.0f;
	ME(1,1) = 1.0f;
	ME(1,2) = 0.0f;
	ME(2,0) = -sintheta;
	ME(2,1) = 0.0f;
	ME(2,2) = costheta;
}

// loadRotationZ
inline void mat3_t::SetRotationZ( float rad )
{
	float sintheta, costheta;
	SinCos( rad, sintheta, costheta );

	ME(0,0) = costheta;
	ME(0,1) = -sintheta;
	ME(0,2) = 0.0f;
	ME(1,0) = sintheta;
	ME(1,1) = costheta;
	ME(1,2) = 0.0f;
	ME(2,0) = 0.0f;
	ME(2,1) = 0.0f;
	ME(2,2) = 1.0f;
}

// RotateXAxis
/* the slow code is in comments and above the comments the optimized one
If we analize the mat3 we can extract the 3 unit vectors rotated by the mat3. The 3 rotated vectors are in mat's colomns.
This means that: mat3.colomn[0] == i*mat3. RotateXAxis() rotates rad angle not from i vector (aka x axis) but
from the vector from colomn 0*/
inline void mat3_t::RotateXAxis( float rad )
{
	float sina, cosa;
	SinCos( rad, sina, cosa );

	/*vec3_t x_axis, y_axis, z_axis;
	GetColumns( x_axis, y_axis, z_axis );*/

	// z_axis = z_axis*cosa - y_axis*sina;
	ME(0,2) = ME(0,2)*cosa - ME(0,1)*sina;
	ME(1,2) = ME(1,2)*cosa - ME(1,1)*sina;
	ME(2,2) = ME(2,2)*cosa - ME(2,1)*sina;

	// z_axis.Normalize();
	float len = InvSqrt( ME(0,2)*ME(0,2) + ME(1,2)*ME(1,2) + ME(2,2)*ME(2,2) );
	ME(0,2) *= len;
	ME(1,2) *= len;
	ME(2,2) *= len;

	// y_axis = z_axis * x_axis;
	ME(0,1) = ME(1,2)*ME(2,0) - ME(2,2)*ME(1,0);
	ME(1,1) = ME(2,2)*ME(0,0) - ME(0,2)*ME(2,0);
	ME(2,1) = ME(0,2)*ME(1,0) - ME(1,2)*ME(0,0);

	// y_axis.Normalize();
	/*len = InvSqrt( ME(0,1)*ME(0,1) + ME(1,1)*ME(1,1) + ME(2,1)*ME(2,1) );
	ME(0,1) *= len;
	ME(1,1) *= len;
	ME(2,1) *= len;*/

	// SetColumns( x_axis, y_axis, z_axis );

}

// RotateYAxis
inline void mat3_t::RotateYAxis( float rad )
{
	float sina, cosa;
	SinCos( rad, sina, cosa );

	/*vec3_t x_axis, y_axis, z_axis;
	GetColumns( x_axis, y_axis, z_axis );*/

	// z_axis = z_axis*cosa + x_axis*sina;
	ME(0,2) = ME(0,2)*cosa + ME(0,0)*sina;
	ME(1,2) = ME(1,2)*cosa + ME(1,0)*sina;
	ME(2,2) = ME(2,2)*cosa + ME(2,0)*sina;

	// z_axis.Normalize();
	float len = InvSqrt( ME(0,2)*ME(0,2) + ME(1,2)*ME(1,2) + ME(2,2)*ME(2,2) );
	ME(0,2) *= len;
	ME(1,2) *= len;
	ME(2,2) *= len;

	// x_axis = (z_axis*y_axis) * -1.0f;
	ME(0,0) = ME(2,2)*ME(1,1) - ME(1,2)*ME(2,1);
	ME(1,0) = ME(0,2)*ME(2,1) - ME(2,2)*ME(0,1);
	ME(2,0) = ME(1,2)*ME(0,1) - ME(0,2)*ME(1,1);

	// x_axis.Normalize();
	/*len = InvSqrt( ME(0,0)*ME(0,0) + ME(1,0)*ME(1,0) + ME(2,0)*ME(2,0) );
	ME(0,0) *= len;
	ME(1,0) *= len;
	ME(2,0) *= len;*/

	// SetColumns( x_axis, y_axis, z_axis );
}


// RotateZAxis
inline void mat3_t::RotateZAxis( float rad )
{
	float sina, cosa;
	SinCos( rad, sina, cosa );

	/*vec3_t x_axis, y_axis, z_axis;
	GetColumns( x_axis, y_axis, z_axis );*/

	// x_axis = x_axis*cosa + y_axis*sina;
	ME(0,0) = ME(0,0)*cosa + ME(0,1)*sina;
	ME(1,0) = ME(1,0)*cosa + ME(1,1)*sina;
	ME(2,0) = ME(2,0)*cosa + ME(2,1)*sina;

	// x_axis.Normalize();
	float len = InvSqrt( ME(0,0)*ME(0,0) + ME(1,0)*ME(1,0) + ME(2,0)*ME(2,0) );
	ME(0,0) *= len;
	ME(1,0) *= len;
	ME(2,0) *= len;

	// y_axis = z_axis*x_axis;
	ME(0,1) = ME(1,2)*ME(2,0) - ME(2,2)*ME(1,0);
	ME(1,1) = ME(2,2)*ME(0,0) - ME(0,2)*ME(2,0);
	ME(2,1) = ME(0,2)*ME(1,0) - ME(1,2)*ME(0,0);

	// y_axis.Normalize();
	/*len = InvSqrt( ME(0,1)*ME(0,1) + ME(1,1)*ME(1,1) + ME(2,1)*ME(2,1) );
	ME(0,1) *= len;
	ME(1,1) *= len;
	ME(2,1) *= len;*/

	//SetColumns( x_axis, y_axis, z_axis );
}

// Transpose
inline void mat3_t::Transpose()
{
	float temp = ME(0,1);
	ME(0,1) = ME(1,0);
	ME(1,0) = temp;
	temp = ME(0,2);
	ME(0,2) = ME(2,0);
	ME(2,0) = temp;
	temp = ME(1,2);
	ME(1,2) = ME(2,1);
	ME(2,1) = temp;
}

// Transposed
inline mat3_t mat3_t::GetTransposed() const
{
	mat3_t m3;
	m3[0] = ME[0];
	m3[1] = ME[3];
	m3[2] = ME[6];
	m3[3] = ME[1];
	m3[4] = ME[4];
	m3[5] = ME[7];
	m3[6] = ME[2];
	m3[7] = ME[5];
	m3[8] = ME[8];
	return m3;
}

// Reorthogonalize
inline void mat3_t::Reorthogonalize()
{
	// method 1: standard orthogonalization method
	/*mat3_t correction_m3 =
	(
		(mat3_t::ident * 3.0f) -
		(ME * ME.Transposed())
	) * 0.5f;

	ME = correction_m3 * ME;*/

	// method 2: Gram-Schmidt method with a twist for z_axis
	vec3_t x_axis, y_axis, z_axis;
	GetColumns( x_axis, y_axis, z_axis );

	x_axis.Normalize();

	y_axis = y_axis - ( x_axis * x_axis.Dot(y_axis) );
	y_axis.Normalize();

	z_axis = x_axis.Cross(y_axis);

	SetColumns( x_axis, y_axis, z_axis );
}

// Print
inline void mat3_t::Print() const
{
	for( int i=0; i<3; i++ )
	{
		for( int j=0; j<3; j++ )
			cout << fixed << ME(i, j) << " ";
		cout << endl;
	}
	cout << endl;
}

// Determinant
inline float mat3_t::Det() const
{
	/* accurate method:
	return ME(0, 0)*ME(1, 1)*ME(2, 2) + ME(0, 1)*ME(1, 2)*ME(2, 0) + ME(0, 2)*ME(1, 0)*ME(2, 1)
	- ME(0, 0)*ME(1, 2)*ME(2, 1) - ME(0, 1)*ME(1, 0)*ME(2, 2) - ME(0, 2)*ME(1, 1)*ME(2, 0);*/
	return ME(0, 0)*( ME(1, 1)*ME(2, 2) - ME(1, 2)*ME(2, 1) ) -
	ME(0, 1)*( ME(1, 0)*ME(2, 2) - ME(1, 2)*ME(2, 0) ) +
	ME(0, 2)*( ME(0, 1)*ME(2, 1) - ME(1, 1)*ME(2, 0) );
}

// GetInverse
// using Gramer's method ( Inv(A) = ( 1/Det(A) ) * Adj(A)  )
inline mat3_t mat3_t::GetInverse() const
{
	mat3_t result;

	// compute determinant
	float cofactor0 = ME(1,1)*ME(2,2) - ME(1,2)*ME(2,1);
	float cofactor3 = ME(0,2)*ME(2,1) - ME(0,1)*ME(2,2);
	float cofactor6 = ME(0,1)*ME(1,2) - ME(0,2)*ME(1,1);
	float det = ME(0,0)*cofactor0 + ME(1,0)*cofactor3 + ME(2,0)*cofactor6;

	DEBUG_ERR( IsZero( det ) ); // Cannot invert det == 0

	// create adjoint matrix and multiply by 1/det to get inverse
	float invDet = 1.0f/det;
	result(0,0) = invDet*cofactor0;
	result(0,1) = invDet*cofactor3;
	result(0,2) = invDet*cofactor6;

	result(1,0) = invDet*(ME(1,2)*ME(2,0) - ME(1,0)*ME(2,2));
	result(1,1) = invDet*(ME(0,0)*ME(2,2) - ME(0,2)*ME(2,0));
	result(1,2) = invDet*(ME(0,2)*ME(1,0) - ME(0,0)*ME(1,2));

	result(2,0) = invDet*(ME(1,0)*ME(2,1) - ME(1,1)*ME(2,0));
	result(2,1) = invDet*(ME(0,1)*ME(2,0) - ME(0,0)*ME(2,1));
	result(2,2) = invDet*(ME(0,0)*ME(1,1) - ME(0,1)*ME(1,0));

	return result;
}

// Invert
// see above
inline void mat3_t::Invert()
{
	ME = GetInverse();
}

// GetZero
inline const mat3_t& mat3_t::GetZero()
{
	static mat3_t zero( 0.0 );
	return zero;
}

// GetIdentity
inline const mat3_t& mat3_t::GetIdentity()
{
	static mat3_t ident( 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 );
	return ident;
}

} // end namespace
