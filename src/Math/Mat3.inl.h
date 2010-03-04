#include "MathDfltHeader.h"


#define ME (*this)


namespace M {

// accessors
inline float& Mat3::operator ()( const uint i, const uint j )
{
	return arr2[i][j];
}

inline const float& Mat3::operator ()( const uint i, const uint j ) const
{
	return arr2[i][j]; 
}

inline float& Mat3::operator []( const uint i)
{
	return arr1[i];
}

inline const float& Mat3::operator []( const uint i) const
{
	return arr1[i];
}

// constructor [float[]]
inline Mat3::Mat3( float arr [] )
{
	for( int i=0; i<9; i++ )
		ME[i] = arr[i];
}

// constructor [float...........float]
inline Mat3::Mat3( float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22 )
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
inline Mat3::Mat3( const Mat3& b )
{
	for( int i=0; i<9; i++ )
		ME[i] = b[i];
}

// constructor [quat]
inline Mat3::Mat3( const Quat& q )
{
	DEBUG_ERR( !isZero( 1.0f - q.getLength()) ); // Not normalized quat

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
	ME(1,1) = 1.0 - (xx + zz);
	ME(1,2) = yz - wx;

	ME(2,0) = xz - wy;
	ME(2,1) = yz + wx;
	ME(2,2) = 1.0 - (xx + yy);
}

// constructor [euler]
inline Mat3::Mat3( const Euler& e )
{
	float ch, sh, ca, sa, cb, sb;
  sinCos( e.heading(), sh, ch );
  sinCos( e.attitude(), sa, ca );
  sinCos( e.bank(), sb, cb );

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
inline Mat3::Mat3( const Axisang& axisang )
{
	DEBUG_ERR( !isZero( 1.0-axisang.axis.getLength() ) ); // Not normalized axis

	float c, s;
	sinCos( axisang.ang, s, c );
	float t = 1.0 - c;

	const Vec3& axis = axisang.axis;
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
inline Mat3::Mat3( float f )
{
	for( int i=0; i<9; i++ )
			ME[i] = f;
}

// 3x3 + 3x3
inline Mat3 Mat3::operator +( const Mat3& b ) const
{
	Mat3 c;
	for( int i=0; i<9; i++ )
		c[i] = ME[i] + b[i];
	return c;
}

// 3x3 += 3x3
inline Mat3& Mat3::operator +=( const Mat3& b )
{
	for( int i=0; i<9; i++ )
		ME[i] += b[i];
	return ME;
}

// 3x3 - 3x3
inline Mat3 Mat3::operator -( const Mat3& b ) const
{
	Mat3 c;
	for( int i=0; i<9; i++ )
		c[i] = ME[i] - b[i];
	return c;
}

// 3x3 -= 3x3
inline Mat3& Mat3::operator -=( const Mat3& b )
{
	for( int i=0; i<9; i++ )
		ME[i] -= b[i];
	return ME;
}

// 3x3 * 3x3
inline Mat3 Mat3::operator *( const Mat3& b ) const
{
	Mat3 c;
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
inline Mat3& Mat3::operator *=( const Mat3& b )
{
	ME = ME * b;
	return ME;
}

// 3x3 + float
inline Mat3 Mat3::operator +( float f ) const
{
	Mat3 c;
	for( uint i=0; i<9; i++ )
		c[i] = ME[i] + f;
	return c;
}

// float + 3x3
inline Mat3 operator +( float f, const Mat3& m3 )
{
	return m3+f;
}

// 3x3 += float
inline Mat3& Mat3::operator +=( float f )
{
	for( uint i=0; i<9; i++ )
		ME[i] += f;
	return ME;
}

// 3x3 - float
inline Mat3 Mat3::operator -( float f ) const
{
	Mat3 c;
	for( uint i=0; i<9; i++ )
		c[i] = ME[i] - f;
	return c;
}

// float - 3x3
inline Mat3 operator -( float f, const Mat3& m3 )
{
	Mat3 out;
	for( uint i=0; i<9; i++ )
		out[i] = f - m3[i];
	return out;
}

// 3x3 -= float
inline Mat3& Mat3::operator -=( float f )
{
	for( uint i=0; i<9; i++ )
		ME[i] -= f;
	return ME;
}

// 3x3 * float
inline Mat3 Mat3::operator *( float f ) const
{
	Mat3 c;
	for( uint i=0; i<9; i++ )
		c[i] = ME[i] * f;
	return c;
}

// float * 3x3
inline Mat3 operator *( float f, const Mat3& m3 )
{
	Mat3 out;
	for( uint i=0; i<9; i++ )
		out[i] = f * m3[i];
	return out;
}

// 3x3 *= float
inline Mat3& Mat3::operator *=( float f )
{
	for( uint i=0; i<9; i++ )
		ME[i] *= f;
	return ME;
}

// 3x3 / float
inline Mat3 Mat3::operator /( float f ) const
{
	Mat3 c;
	for( uint i=0; i<9; i++ )
		c[i] = ME[i] / f;
	return c;
}

// float / 3x3
inline Mat3 operator /( float f, const Mat3& m3 )
{
	Mat3 out;
	for( uint i=0; i<9; i++ )
		out[i] = f / m3[i];
	return out;
}

// 3x3 / float (self)
inline Mat3& Mat3::operator /=( float f )
{
	for( uint i=0; i<9; i++ )
		ME[i] /= f;
	return ME;
}

// 3x3 * vec3 (cross products with rows)
inline Vec3 Mat3::operator *( const Vec3& b ) const
{
	return Vec3(
		ME(0, 0)*b.x + ME(0, 1)*b.y + ME(0, 2)*b.z,
		ME(1, 0)*b.x + ME(1, 1)*b.y + ME(1, 2)*b.z,
		ME(2, 0)*b.x + ME(2, 1)*b.y + ME(2, 2)*b.z
	);
}

// ==
inline bool Mat3::operator ==( const Mat3& b ) const
{
	for( int i=0; i<9; i++ )
		if( !isZero( ME[i]-b[i] ) ) return false;
	return true;
}

// !=
inline bool Mat3::operator !=( const Mat3& b ) const
{
	for( int i=0; i<9; i++ )
		if( !isZero( ME[i]-b[i] ) ) return true;
	return false;
}

// setRows
inline void Mat3::setRows( const Vec3& a, const Vec3& b, const Vec3& c )
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

// setColumns
inline void Mat3::setColumns( const Vec3& a, const Vec3& b, const Vec3& c )
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

// getRows
inline void Mat3::getRows( Vec3& a, Vec3& b, Vec3& c ) const
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

// getColumns
inline void Mat3::getColumns( Vec3& a, Vec3& b, Vec3& c ) const
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

// setRow
inline void Mat3::setRow( const uint i, const Vec3& v )
{
	ME(i,0)=v.x;
	ME(i,1)=v.y;
	ME(i,2)=v.z;
}

// getRow
inline Vec3 Mat3::getRow( const uint i ) const
{
	return Vec3( ME(i,0), ME(i,1), ME(i,2) );
}

// setColumn
inline void Mat3::setColumn( const uint i, const Vec3& v )
{
	ME(0,i)=v.x;
	ME(1,i)=v.y;
	ME(2,i)=v.z;
}

// getColumn
inline Vec3 Mat3::getColumn( const uint i ) const
{
	return Vec3( ME(0,i), ME(1,i), ME(2,i) );
}

// setRotationX
inline void Mat3::setRotationX( float rad )
{
	float sintheta, costheta;
	sinCos( rad, sintheta, costheta );

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

// setRotationY
inline void Mat3::setRotationY( float rad )
{
	float sintheta, costheta;
	sinCos( rad, sintheta, costheta );

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
inline void Mat3::setRotationZ( float rad )
{
	float sintheta, costheta;
	sinCos( rad, sintheta, costheta );

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

// rotateXAxis
/* the slow code is in comments and above the comments the optimized one
If we analize the mat3 we can extract the 3 unit vectors rotated by the mat3. The 3 rotated vectors are in mat's colomns.
This means that: mat3.colomn[0] == i*mat3. rotateXAxis() rotates rad angle not from i vector (aka x axis) but
from the vector from colomn 0*/
inline void Mat3::rotateXAxis( float rad )
{
	float sina, cosa;
	sinCos( rad, sina, cosa );

	/*Vec3 xAxis, yAxis, zAxis;
	getColumns( xAxis, yAxis, zAxis );*/

	// zAxis = zAxis*cosa - yAxis*sina;
	ME(0,2) = ME(0,2)*cosa - ME(0,1)*sina;
	ME(1,2) = ME(1,2)*cosa - ME(1,1)*sina;
	ME(2,2) = ME(2,2)*cosa - ME(2,1)*sina;

	// zAxis.normalize();
	float len = invSqrt( ME(0,2)*ME(0,2) + ME(1,2)*ME(1,2) + ME(2,2)*ME(2,2) );
	ME(0,2) *= len;
	ME(1,2) *= len;
	ME(2,2) *= len;

	// yAxis = zAxis * xAxis;
	ME(0,1) = ME(1,2)*ME(2,0) - ME(2,2)*ME(1,0);
	ME(1,1) = ME(2,2)*ME(0,0) - ME(0,2)*ME(2,0);
	ME(2,1) = ME(0,2)*ME(1,0) - ME(1,2)*ME(0,0);

	// yAxis.normalize();
	/*len = invSqrt( ME(0,1)*ME(0,1) + ME(1,1)*ME(1,1) + ME(2,1)*ME(2,1) );
	ME(0,1) *= len;
	ME(1,1) *= len;
	ME(2,1) *= len;*/

	// setColumns( xAxis, yAxis, zAxis );

}

// rotateYAxis
inline void Mat3::rotateYAxis( float rad )
{
	float sina, cosa;
	sinCos( rad, sina, cosa );

	/*Vec3 xAxis, yAxis, zAxis;
	getColumns( xAxis, yAxis, zAxis );*/

	// zAxis = zAxis*cosa + xAxis*sina;
	ME(0,2) = ME(0,2)*cosa + ME(0,0)*sina;
	ME(1,2) = ME(1,2)*cosa + ME(1,0)*sina;
	ME(2,2) = ME(2,2)*cosa + ME(2,0)*sina;

	// zAxis.normalize();
	float len = invSqrt( ME(0,2)*ME(0,2) + ME(1,2)*ME(1,2) + ME(2,2)*ME(2,2) );
	ME(0,2) *= len;
	ME(1,2) *= len;
	ME(2,2) *= len;

	// xAxis = (zAxis*yAxis) * -1.0f;
	ME(0,0) = ME(2,2)*ME(1,1) - ME(1,2)*ME(2,1);
	ME(1,0) = ME(0,2)*ME(2,1) - ME(2,2)*ME(0,1);
	ME(2,0) = ME(1,2)*ME(0,1) - ME(0,2)*ME(1,1);

	// xAxis.normalize();
	/*len = invSqrt( ME(0,0)*ME(0,0) + ME(1,0)*ME(1,0) + ME(2,0)*ME(2,0) );
	ME(0,0) *= len;
	ME(1,0) *= len;
	ME(2,0) *= len;*/

	// setColumns( xAxis, yAxis, zAxis );
}


// rotateZAxis
inline void Mat3::rotateZAxis( float rad )
{
	float sina, cosa;
	sinCos( rad, sina, cosa );

	/*Vec3 xAxis, yAxis, zAxis;
	getColumns( xAxis, yAxis, zAxis );*/

	// xAxis = xAxis*cosa + yAxis*sina;
	ME(0,0) = ME(0,0)*cosa + ME(0,1)*sina;
	ME(1,0) = ME(1,0)*cosa + ME(1,1)*sina;
	ME(2,0) = ME(2,0)*cosa + ME(2,1)*sina;

	// xAxis.normalize();
	float len = invSqrt( ME(0,0)*ME(0,0) + ME(1,0)*ME(1,0) + ME(2,0)*ME(2,0) );
	ME(0,0) *= len;
	ME(1,0) *= len;
	ME(2,0) *= len;

	// yAxis = zAxis*xAxis;
	ME(0,1) = ME(1,2)*ME(2,0) - ME(2,2)*ME(1,0);
	ME(1,1) = ME(2,2)*ME(0,0) - ME(0,2)*ME(2,0);
	ME(2,1) = ME(0,2)*ME(1,0) - ME(1,2)*ME(0,0);

	// yAxis.normalize();
	/*len = invSqrt( ME(0,1)*ME(0,1) + ME(1,1)*ME(1,1) + ME(2,1)*ME(2,1) );
	ME(0,1) *= len;
	ME(1,1) *= len;
	ME(2,1) *= len;*/

	//setColumns( xAxis, yAxis, zAxis );
}

// transpose
inline void Mat3::transpose()
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

// transposed
inline Mat3 Mat3::getTransposed() const
{
	Mat3 m3;
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

// reorthogonalize
inline void Mat3::reorthogonalize()
{
	// method 1: standard orthogonalization method
	/*Mat3 correction_m3 =
	(
		(Mat3::ident * 3.0f) -
		(ME * ME.transposed())
	) * 0.5f;

	ME = correction_m3 * ME;*/

	// method 2: Gram-Schmidt method with a twist for zAxis
	Vec3 xAxis, yAxis, zAxis;
	getColumns( xAxis, yAxis, zAxis );

	xAxis.normalize();

	yAxis = yAxis - ( xAxis * xAxis.dot(yAxis) );
	yAxis.normalize();

	zAxis = xAxis.cross(yAxis);

	setColumns( xAxis, yAxis, zAxis );
}

// print
inline void Mat3::print() const
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
inline float Mat3::getDet() const
{
	/* accurate method:
	return ME(0, 0)*ME(1, 1)*ME(2, 2) + ME(0, 1)*ME(1, 2)*ME(2, 0) + ME(0, 2)*ME(1, 0)*ME(2, 1)
	- ME(0, 0)*ME(1, 2)*ME(2, 1) - ME(0, 1)*ME(1, 0)*ME(2, 2) - ME(0, 2)*ME(1, 1)*ME(2, 0);*/
	return ME(0, 0)*( ME(1, 1)*ME(2, 2) - ME(1, 2)*ME(2, 1) ) -
	ME(0, 1)*( ME(1, 0)*ME(2, 2) - ME(1, 2)*ME(2, 0) ) +
	ME(0, 2)*( ME(0, 1)*ME(2, 1) - ME(1, 1)*ME(2, 0) );
}

// getInverse
// using Gramer's method ( Inv(A) = ( 1/getDet(A) ) * Adj(A)  )
inline Mat3 Mat3::getInverse() const
{
	Mat3 result;

	// compute determinant
	float cofactor0 = ME(1,1)*ME(2,2) - ME(1,2)*ME(2,1);
	float cofactor3 = ME(0,2)*ME(2,1) - ME(0,1)*ME(2,2);
	float cofactor6 = ME(0,1)*ME(1,2) - ME(0,2)*ME(1,1);
	float det = ME(0,0)*cofactor0 + ME(1,0)*cofactor3 + ME(2,0)*cofactor6;

	DEBUG_ERR( isZero( det ) ); // Cannot invert det == 0

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

// invert
// see above
inline void Mat3::invert()
{
	ME = getInverse();
}

// getZero
inline const Mat3& Mat3::getZero()
{
	static Mat3 zero( 0.0 );
	return zero;
}

// getIdentity
inline const Mat3& Mat3::getIdentity()
{
	static Mat3 ident( 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 );
	return ident;
}

} // end namespace
