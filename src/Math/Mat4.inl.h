#include "MathDfltHeader.h"


#define ME (*this)


namespace M {

// accessors
inline float& Mat4::operator ()( const uint i, const uint j )
{
	return arr2[i][j];
}

inline const float& Mat4::operator ()( const uint i, const uint j ) const
{
	return arr2[i][j];
}

inline float& Mat4::operator []( const uint i) 
{ 
	return arr1[i]; 
}

inline const float& Mat4::operator []( const uint i) const 
{ 
	return arr1[i]; 
}

// constructor [mat4]
inline Mat4::Mat4( const Mat4& b )
{
	for( int i=0; i<16; i++ )
		ME[i] = b[i];
}

// constructor [float[]]
inline Mat4::Mat4( const float arr_ [] )
{
	for( int i=0; i<16; i++ )
		ME[i] = arr_[i];
}

// constructor [float..........]
inline Mat4::Mat4( float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33 )
{
	ME(0,0) = m00;
	ME(0,1) = m01;
	ME(0,2) = m02;
	ME(0,3) = m03;
	ME(1,0) = m10;
	ME(1,1) = m11;
	ME(1,2) = m12;
	ME(1,3) = m13;
	ME(2,0) = m20;
	ME(2,1) = m21;
	ME(2,2) = m22;
	ME(2,3) = m23;
	ME(3,0) = m30;
	ME(3,1) = m31;
	ME(3,2) = m32;
	ME(3,3) = m33;
}

// constructor [mat3]
inline Mat4::Mat4( const Mat3& m3 )
{
	ME(0,0) = m3(0,0);
	ME(0,1) = m3(0,1);
	ME(0,2) = m3(0,2);
	ME(1,0) = m3(1,0);
	ME(1,1) = m3(1,1);
	ME(1,2) = m3(1,2);
	ME(2,0) = m3(2,0);
	ME(2,1) = m3(2,1);
	ME(2,2) = m3(2,2);
	ME(3, 0) = ME(3, 1) = ME(3, 2) = ME(0, 3) = ME(1, 3) = ME(2, 3) = 0.0;
	ME(3, 3) = 1.0;
}

// constructor [vec3]
inline Mat4::Mat4( const Vec3& v )
{
	ME(0, 0) = 1.0;
	ME(0, 1) = 0.0;
	ME(0, 2) = 0.0;
	ME(0, 3) = v.x;
	ME(1, 0) = 0.0;
	ME(1, 1) = 1.0;
	ME(1, 2) = 0.0;
	ME(1, 3) = v.y;
	ME(2, 0) = 0.0;
	ME(2, 1) = 0.0;
	ME(2, 2) = 1.0;
	ME(2, 3) = v.z;
	ME(3, 0) = 0.0;
	ME(3, 1) = 0.0;
	ME(3, 2) = 0.0;
	ME(3, 3) = 1.0;
}

// constructor [vec4]
inline Mat4::Mat4( const Vec4& v )
{
	ME(0, 0) = 1.0;
	ME(0, 1) = 0.0;
	ME(0, 2) = 0.0;
	ME(0, 3) = v.x;
	ME(1, 0) = 0.0;
	ME(1, 1) = 1.0;
	ME(1, 2) = 0.0;
	ME(1, 3) = v.y;
	ME(2, 0) = 0.0;
	ME(2, 1) = 0.0;
	ME(2, 2) = 1.0;
	ME(2, 3) = v.z;
	ME(3, 0) = 0.0;
	ME(3, 1) = 0.0;
	ME(3, 2) = 0.0;
	ME(3, 3) = v.w;
}

// constructor [vec3, mat3]
inline Mat4::Mat4( const Vec3& transl, const Mat3& rot )
{
	setRotationPart(rot);
	setTranslationPart(transl);
	ME(3,0) = ME(3,1) = ME(3,2) = 0.0;
	ME(3,3) = 1.0;
}

// constructor [vec3, mat3, float]
inline Mat4::Mat4( const Vec3& translate, const Mat3& rotate, float scale )
{
	if( !isZero( scale-1.0 ) )
		setRotationPart( rotate*scale );
	else
		setRotationPart( rotate );

	setTranslationPart( translate );

	ME(3,0) = ME(3,1) = ME(3,2) = 0.0;
	ME(3,3) = 1.0;
}

// constructor [float]
inline Mat4::Mat4( float f )
{
	for( int i=0; i<16; i++ )
		ME[i] = f;
}

// constructor [Transform]
inline Mat4::Mat4( const Transform& t )
{
	ME = Mat4( t.getOrigin(), t.getRotation(), t.getScale() );
}

// 4x4 + 4x4
inline Mat4 Mat4::operator +( const Mat4& b ) const
{
	Mat4 c;
	for( int i=0; i<16; i++ )
		c[i] = ME[i] + b[i];
	return c;
}

// 4x4 + 4x4 (self)
inline Mat4& Mat4::operator +=( const Mat4& b )
{
	for( int i=0; i<16; i++ )
		ME[i] += b[i];
	return ME;
}

// 4x4 - 4x4
inline Mat4 Mat4::operator -( const Mat4& b ) const
{
	Mat4 c;
	for( int i=0; i<16; i++ )
		c[i] = ME[i] - b[i];
	return c;
}

// 4x4 - 4x4 (self)
inline Mat4& Mat4::operator -=( const Mat4& b )
{
	for( int i=0; i<16; i++ )
		ME[i] -= b[i];
	return ME;
}

// 4x4 * 4x4
inline Mat4 Mat4::operator *( const Mat4& b ) const
{
	Mat4 c;
	c(0,0) = ME(0,0)*b(0,0) + ME(0,1)*b(1,0) + ME(0,2)*b(2,0) + ME(0,3)*b(3,0);
	c(0,1) = ME(0,0)*b(0,1) + ME(0,1)*b(1,1) + ME(0,2)*b(2,1) + ME(0,3)*b(3,1);
	c(0,2) = ME(0,0)*b(0,2) + ME(0,1)*b(1,2) + ME(0,2)*b(2,2) + ME(0,3)*b(3,2);
	c(0,3) = ME(0,0)*b(0,3) + ME(0,1)*b(1,3) + ME(0,2)*b(2,3) + ME(0,3)*b(3,3);
	c(1,0) = ME(1,0)*b(0,0) + ME(1,1)*b(1,0) + ME(1,2)*b(2,0) + ME(1,3)*b(3,0);
	c(1,1) = ME(1,0)*b(0,1) + ME(1,1)*b(1,1) + ME(1,2)*b(2,1) + ME(1,3)*b(3,1);
	c(1,2) = ME(1,0)*b(0,2) + ME(1,1)*b(1,2) + ME(1,2)*b(2,2) + ME(1,3)*b(3,2);
	c(1,3) = ME(1,0)*b(0,3) + ME(1,1)*b(1,3) + ME(1,2)*b(2,3) + ME(1,3)*b(3,3);
	c(2,0) = ME(2,0)*b(0,0) + ME(2,1)*b(1,0) + ME(2,2)*b(2,0) + ME(2,3)*b(3,0);
	c(2,1) = ME(2,0)*b(0,1) + ME(2,1)*b(1,1) + ME(2,2)*b(2,1) + ME(2,3)*b(3,1);
	c(2,2) = ME(2,0)*b(0,2) + ME(2,1)*b(1,2) + ME(2,2)*b(2,2) + ME(2,3)*b(3,2);
	c(2,3) = ME(2,0)*b(0,3) + ME(2,1)*b(1,3) + ME(2,2)*b(2,3) + ME(2,3)*b(3,3);
	c(3,0) = ME(3,0)*b(0,0) + ME(3,1)*b(1,0) + ME(3,2)*b(2,0) + ME(3,3)*b(3,0);
	c(3,1) = ME(3,0)*b(0,1) + ME(3,1)*b(1,1) + ME(3,2)*b(2,1) + ME(3,3)*b(3,1);
	c(3,2) = ME(3,0)*b(0,2) + ME(3,1)*b(1,2) + ME(3,2)*b(2,2) + ME(3,3)*b(3,2);
	c(3,3) = ME(3,0)*b(0,3) + ME(3,1)*b(1,3) + ME(3,2)*b(2,3) + ME(3,3)*b(3,3);
	return c;
}

// 4x4 * 4x4 (self)
inline Mat4& Mat4::operator *=( const Mat4& b )
{
	ME = ME * b;
	return ME;
}

// ==
inline bool Mat4::operator ==( const Mat4& b ) const
{
	for( int i=0; i<16; i++ )
		if( !isZero( ME[i]-b[i] ) ) return false;
	return true;
}

// !=
inline bool Mat4::operator !=( const Mat4& b ) const
{
	for( int i=0; i<16; i++ )
		if( !isZero( ME[i]-b[i] ) ) return true;
	return false;
}

// 4x4 * vec4
inline Vec4 Mat4::operator *( const Vec4& b ) const
{
	return Vec4(
		ME(0,0)*b.x + ME(0,1)*b.y + ME(0,2)*b.z + ME(0,3)*b.w,
		ME(1,0)*b.x + ME(1,1)*b.y + ME(1,2)*b.z + ME(1,3)*b.w,
		ME(2,0)*b.x + ME(2,1)*b.y + ME(2,2)*b.z + ME(2,3)*b.w,
		ME(3,0)*b.x + ME(3,1)*b.y + ME(3,2)*b.z + ME(3,3)*b.w
	);
}

// 4x4 + float
inline Mat4 Mat4::operator +( float f ) const
{
	Mat4 c;
	for( int i=0; i<16; i++ )
		c[i] = ME[i] + f;
	return c;
}

// float + 4x4
inline Mat4 operator +( float f, const Mat4& m4 )
{
	return m4+f;
}

// 4x4 + float (self)
inline Mat4& Mat4::operator +=( float f )
{
	for( int i=0; i<16; i++ )
		ME[i] += f;
	return ME;
}

// 4x4 - float
inline Mat4 Mat4::operator -( float f ) const
{
	Mat4 c;
	for( int i=0; i<16; i++ )
		c[i] = ME[i] - f;
	return c;
}

// float - 4x4
inline Mat4 operator -( float f, const Mat4& m4 )
{
	Mat4 out;
	for( int i=0; i<16; i++ )
		out[i] = f- m4[i];
	return out;
}

// 4x4 - float (self)
inline Mat4& Mat4::operator -=( float f )
{
	for( int i=0; i<16; i++ )
		ME[i] -= f;
	return ME;
}

// 4x4 * float
inline Mat4 Mat4::operator *( float f ) const
{
	Mat4 c;
	for( int i=0; i<16; i++ )
		c[i] = ME[i] * f;
	return c;
}

// float * 4x4
inline Mat4 operator *( float f, const Mat4& m4 )
{
	return m4*f;
}

// 4x4 *= float
inline Mat4& Mat4::operator *=( float f )
{
	for( int i=0; i<16; i++ )
		ME[i] *= f;
	return ME;
}

// 4x4 / float
inline Mat4 Mat4::operator /( float f ) const
{
	Mat4 c;
	for( int i=0; i<16; i++ )
		c[i] = ME[i] / f;
	return c;
}

// float / 4x4
inline Mat4 operator /( float f, const Mat4& m4 )
{
	Mat4 out;
	for( uint i=0; i<9; i++ )
		out[i] = f / m4[i];
	return out;
}

// 4x4 /= float
inline Mat4& Mat4::operator /=( float f )
{
	for( int i=0; i<16; i++ )
		ME[i] /= f;
	return ME;
}

// setRows
inline void Mat4::setRows( const Vec4& a, const Vec4& b, const Vec4& c, const Vec4& d )
{
	ME(0,0) = a.x;
	ME(0,1) = a.y;
	ME(0,2) = a.z;
	ME(0,3) = a.w;
	ME(1,0) = b.x;
	ME(1,1) = b.y;
	ME(1,2) = b.z;
	ME(1,3) = b.w;
	ME(2,0) = c.x;
	ME(2,1) = c.y;
	ME(2,2) = c.z;
	ME(2,3) = c.w;
	ME(3,0) = d.x;
	ME(3,1) = d.y;
	ME(3,2) = d.z;
	ME(3,3) = d.w;
}

// setRow
inline void Mat4::setRow( uint i, const Vec4& v )
{
	DEBUG_ERR( i > 3 );
	ME(i,0) = v.x;
	ME(i,1) = v.y;
	ME(i,2) = v.z;
	ME(i,3) = v.w;
}

// setColumns
inline void Mat4::setColumns( const Vec4& a, const Vec4& b, const Vec4& c, const Vec4& d )
{
	ME(0,0) = a.x;
	ME(1,0) = a.y;
	ME(2,0) = a.z;
	ME(3,0) = a.w;
	ME(0,1) = b.x;
	ME(1,1) = b.y;
	ME(2,1) = b.z;
	ME(3,1) = b.w;
	ME(0,2) = c.x;
	ME(1,2) = c.y;
	ME(2,2) = c.z;
	ME(3,2) = c.w;
	ME(0,3) = d.x;
	ME(1,3) = d.y;
	ME(2,3) = d.z;
	ME(3,3) = d.w;
}

// setColumn
inline void Mat4::setColumn( uint i, const Vec4& v )
{
	DEBUG_ERR( i > 3 );
	ME(0,i) = v.x;
	ME(1,i) = v.y;
	ME(2,i) = v.z;
	ME(3,i) = v.w;
}

// transpose
inline void Mat4::transpose()
{
	float tmp = ME(0,1);
	ME(0,1) = ME(1,0);
	ME(1,0) = tmp;
	tmp = ME(0,2);
	ME(0,2) = ME(2,0);
	ME(2,0) = tmp;
	tmp = ME(0,3);
	ME(0,3) = ME(3,0);
	ME(3,0) = tmp;
	tmp = ME(1,2);
	ME(1,2) = ME(2,1);
	ME(2,1) = tmp;
	tmp = ME(1,3);
	ME(1,3) = ME(3,1);
	ME(3,1) = tmp;
	tmp = ME(2,3);
	ME(2,3) = ME(3,2);
	ME(3,2) = tmp;
}

// getTransposed
// return the transposed
inline Mat4 Mat4::getTransposed() const
{
	Mat4 m4;
	m4[0] = ME[0];
	m4[1] = ME[4];
	m4[2] = ME[8];
	m4[3] = ME[12];
	m4[4] = ME[1];
	m4[5] = ME[5];
	m4[6] = ME[9];
	m4[7] = ME[13];
	m4[8] = ME[2];
	m4[9] = ME[6];
	m4[10] = ME[10];
	m4[11] = ME[14];
	m4[12] = ME[3];
	m4[13] = ME[7];
	m4[14] = ME[11];
	m4[15] = ME[15];
	return m4;
}

// setRotationPart
inline void Mat4::setRotationPart( const Mat3& m3 )
{
	ME(0,0) = m3(0,0);
	ME(0,1) = m3(0,1);
	ME(0,2) = m3(0,2);
	ME(1,0) = m3(1,0);
	ME(1,1) = m3(1,1);
	ME(1,2) = m3(1,2);
	ME(2,0) = m3(2,0);
	ME(2,1) = m3(2,1);
	ME(2,2) = m3(2,2);
}

// getRotationPart
inline Mat3 Mat4::getRotationPart() const
{
	Mat3 m3;
	m3(0,0) = ME(0,0);
	m3(0,1) = ME(0,1);
	m3(0,2) = ME(0,2);
	m3(1,0) = ME(1,0);
	m3(1,1) = ME(1,1);
	m3(1,2) = ME(1,2);
	m3(2,0) = ME(2,0);
	m3(2,1) = ME(2,1);
	m3(2,2) = ME(2,2);
	return m3;
}

// setTranslationPart
inline void Mat4::setTranslationPart( const Vec4& v )
{
	ME(0, 3) = v.x;
	ME(1, 3) = v.y;
	ME(2, 3) = v.z;
	ME(3, 3) = v.w;
}

// setTranslationPart
inline void Mat4::setTranslationPart( const Vec3& v )
{
	ME(0, 3) = v.x;
	ME(1, 3) = v.y;
	ME(2, 3) = v.z;
}

// getTranslationPart
inline Vec3 Mat4::getTranslationPart() const
{
	return Vec3( ME(0, 3), ME(1, 3), ME(2, 3) );
}

// getIdentity
inline const Mat4& Mat4::getIdentity()
{
	static Mat4 ident( 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0 );
	return ident;
}

// getZero
inline const Mat4& Mat4::getZero()
{
	static Mat4 zero( 0.0 );
	return zero;
}

// print
inline void Mat4::print() const
{
	cout << fixed;
	for( int i=0; i<4; i++ )
	{
		for( int j=0; j<4; j++ )
		{
			if( ME(i, j) < 0.0 )
				cout << ME(i, j) << " ";
			else
				cout << " " << ME(i, j) << " ";
		}
		cout << endl;
	}
	cout << endl;
}

// Determinant
inline float Mat4::getDet() const
{
	return
	ME(0, 3)*ME(1, 2)*ME(2, 1)*ME(3, 0) - ME(0, 2)*ME(1, 3)*ME(2, 1)*ME(3, 0) -
	ME(0, 3)*ME(1, 1)*ME(2, 2)*ME(3, 0) + ME(0, 1)*ME(1, 3)*ME(2, 2)*ME(3, 0) +
	ME(0, 2)*ME(1, 1)*ME(2, 3)*ME(3, 0) - ME(0, 1)*ME(1, 2)*ME(2, 3)*ME(3, 0) -
	ME(0, 3)*ME(1, 2)*ME(2, 0)*ME(3, 1) + ME(0, 2)*ME(1, 3)*ME(2, 0)*ME(3, 1) +
	ME(0, 3)*ME(1, 0)*ME(2, 2)*ME(3, 1) - ME(0, 0)*ME(1, 3)*ME(2, 2)*ME(3, 1) -
	ME(0, 2)*ME(1, 0)*ME(2, 3)*ME(3, 1) + ME(0, 0)*ME(1, 2)*ME(2, 3)*ME(3, 1) +
	ME(0, 3)*ME(1, 1)*ME(2, 0)*ME(3, 2) - ME(0, 1)*ME(1, 3)*ME(2, 0)*ME(3, 2) -
	ME(0, 3)*ME(1, 0)*ME(2, 1)*ME(3, 2) + ME(0, 0)*ME(1, 3)*ME(2, 1)*ME(3, 2) +
	ME(0, 1)*ME(1, 0)*ME(2, 3)*ME(3, 2) - ME(0, 0)*ME(1, 1)*ME(2, 3)*ME(3, 2) -
	ME(0, 2)*ME(1, 1)*ME(2, 0)*ME(3, 3) + ME(0, 1)*ME(1, 2)*ME(2, 0)*ME(3, 3) +
	ME(0, 2)*ME(1, 0)*ME(2, 1)*ME(3, 3) - ME(0, 0)*ME(1, 2)*ME(2, 1)*ME(3, 3) -
	ME(0, 1)*ME(1, 0)*ME(2, 2)*ME(3, 3) + ME(0, 0)*ME(1, 1)*ME(2, 2)*ME(3, 3);
}

// invert
inline void Mat4::invert()
{
	ME = getInverse();
}

// Inverted
inline Mat4 Mat4::getInverse() const
{
	float tmp[12];
	float det;
	const Mat4& in = ME;

	Mat4 m4;


	tmp[0] = in(2,2) * in(3,3);
	tmp[1] = in(3,2) * in(2,3);
	tmp[2] = in(1,2) * in(3,3);
	tmp[3] = in(3,2) * in(1,3);
	tmp[4] = in(1,2) * in(2,3);
	tmp[5] = in(2,2) * in(1,3);
	tmp[6] = in(0,2) * in(3,3);
	tmp[7] = in(3,2) * in(0,3);
	tmp[8] = in(0,2) * in(2,3);
	tmp[9] = in(2,2) * in(0,3);
	tmp[10] = in(0,2) * in(1,3);
	tmp[11] = in(1,2) * in(0,3);

	m4(0,0) =  tmp[0]*in(1,1) + tmp[3]*in(2,1) + tmp[4]*in(3,1);
	m4(0,0) -= tmp[1]*in(1,1) + tmp[2]*in(2,1) + tmp[5]*in(3,1);
	m4(0,1) =  tmp[1]*in(0,1) + tmp[6]*in(2,1) + tmp[9]*in(3,1);
	m4(0,1) -= tmp[0]*in(0,1) + tmp[7]*in(2,1) + tmp[8]*in(3,1);
	m4(0,2) =  tmp[2]*in(0,1) + tmp[7]*in(1,1) + tmp[10]*in(3,1);
	m4(0,2) -= tmp[3]*in(0,1) + tmp[6]*in(1,1) + tmp[11]*in(3,1);
	m4(0,3) =  tmp[5]*in(0,1) + tmp[8]*in(1,1) + tmp[11]*in(2,1);
	m4(0,3) -= tmp[4]*in(0,1) + tmp[9]*in(1,1) + tmp[10]*in(2,1);
	m4(1,0) =  tmp[1]*in(1,0) + tmp[2]*in(2,0) + tmp[5]*in(3,0);
	m4(1,0) -= tmp[0]*in(1,0) + tmp[3]*in(2,0) + tmp[4]*in(3,0);
	m4(1,1) =  tmp[0]*in(0,0) + tmp[7]*in(2,0) + tmp[8]*in(3,0);
	m4(1,1) -= tmp[1]*in(0,0) + tmp[6]*in(2,0) + tmp[9]*in(3,0);
	m4(1,2) =  tmp[3]*in(0,0) + tmp[6]*in(1,0) + tmp[11]*in(3,0);
	m4(1,2) -= tmp[2]*in(0,0) + tmp[7]*in(1,0) + tmp[10]*in(3,0);
	m4(1,3) =  tmp[4]*in(0,0) + tmp[9]*in(1,0) + tmp[10]*in(2,0);
	m4(1,3) -= tmp[5]*in(0,0) + tmp[8]*in(1,0) + tmp[11]*in(2,0);


	tmp[0] = in(2,0)*in(3,1);
	tmp[1] = in(3,0)*in(2,1);
	tmp[2] = in(1,0)*in(3,1);
	tmp[3] = in(3,0)*in(1,1);
	tmp[4] = in(1,0)*in(2,1);
	tmp[5] = in(2,0)*in(1,1);
	tmp[6] = in(0,0)*in(3,1);
	tmp[7] = in(3,0)*in(0,1);
	tmp[8] = in(0,0)*in(2,1);
	tmp[9] = in(2,0)*in(0,1);
	tmp[10] = in(0,0)*in(1,1);
	tmp[11] = in(1,0)*in(0,1);

	m4(2,0) = tmp[0]*in(1,3) + tmp[3]*in(2,3) + tmp[4]*in(3,3);
	m4(2,0)-= tmp[1]*in(1,3) + tmp[2]*in(2,3) + tmp[5]*in(3,3);
	m4(2,1) = tmp[1]*in(0,3) + tmp[6]*in(2,3) + tmp[9]*in(3,3);
	m4(2,1)-= tmp[0]*in(0,3) + tmp[7]*in(2,3) + tmp[8]*in(3,3);
	m4(2,2) = tmp[2]*in(0,3) + tmp[7]*in(1,3) + tmp[10]*in(3,3);
	m4(2,2)-= tmp[3]*in(0,3) + tmp[6]*in(1,3) + tmp[11]*in(3,3);
	m4(2,3) = tmp[5]*in(0,3) + tmp[8]*in(1,3) + tmp[11]*in(2,3);
	m4(2,3)-= tmp[4]*in(0,3) + tmp[9]*in(1,3) + tmp[10]*in(2,3);
	m4(3,0) = tmp[2]*in(2,2) + tmp[5]*in(3,2) + tmp[1]*in(1,2);
	m4(3,0)-= tmp[4]*in(3,2) + tmp[0]*in(1,2) + tmp[3]*in(2,2);
	m4(3,1) = tmp[8]*in(3,2) + tmp[0]*in(0,2) + tmp[7]*in(2,2);
	m4(3,1)-= tmp[6]*in(2,2) + tmp[9]*in(3,2) + tmp[1]*in(0,2);
	m4(3,2) = tmp[6]*in(1,2) + tmp[11]*in(3,2) + tmp[3]*in(0,2);
	m4(3,2)-= tmp[10]*in(3,2) + tmp[2]*in(0,2) + tmp[7]*in(1,2);
	m4(3,3) = tmp[10]*in(2,2) + tmp[4]*in(0,2) + tmp[9]*in(1,2);
	m4(3,3)-= tmp[8]*in(1,2) + tmp[11]*in(2,2) + tmp[5]*in(0,2);

	det = ME(0,0)*m4(0,0)+ME(1,0)*m4(0,1)+ME(2,0)*m4(0,2)+ME(3,0)*m4(0,3);

	DEBUG_ERR( isZero( det ) ); // Cannot invert, det == 0
	det = 1/det;
	m4 *= det;
	return m4;
}


// getInverseTransformation
inline Mat4 Mat4::getInverseTransformation() const
{
	Mat3 invertedRot = (getRotationPart()).getTransposed();
	Vec3 invertedTsl = getTranslationPart();
	invertedTsl = -( invertedRot * invertedTsl );
	return Mat4( invertedTsl, invertedRot );
}

// lerp
inline Mat4 Mat4::lerp( const Mat4& b, float t ) const
{
	return (ME*(1.0-t))+(b*t);
}

// setIdentity
inline void Mat4::setIdentity()
{
	ME = getIdentity();
}

// combineTransformations
inline Mat4 Mat4::combineTransformations( const Mat4& m0, const Mat4& m1 )
{
	/* the clean code is:
	Mat3 rot = m0.getRotationPart() * m1.getRotationPart();  // combine the rotations
	Vec3 tra = (m1.getTranslationPart()).Transformed( m0.getTranslationPart(), m0.getRotationPart(), 1.0 );
	return Mat4( tra, rot );
	and the optimized: */
	DEBUG_ERR( !isZero( m0(3,0)+m0(3,1)+m0(3,2)+m0(3,3)-1.0 ) ||
	           !isZero( m1(3,0)+m1(3,1)+m1(3,2)+m1(3,3)-1.0 ) ); // one of the 2 mat4 doesnt represent transformation

	Mat4 m4;

	m4(0, 0) = m0(0, 0)*m1(0, 0) + m0(0, 1)*m1(1, 0) + m0(0, 2)*m1(2, 0);
	m4(0, 1) = m0(0, 0)*m1(0, 1) + m0(0, 1)*m1(1, 1) + m0(0, 2)*m1(2, 1);
	m4(0, 2) = m0(0, 0)*m1(0, 2) + m0(0, 1)*m1(1, 2) + m0(0, 2)*m1(2, 2);
	m4(1, 0) = m0(1, 0)*m1(0, 0) + m0(1, 1)*m1(1, 0) + m0(1, 2)*m1(2, 0);
	m4(1, 1) = m0(1, 0)*m1(0, 1) + m0(1, 1)*m1(1, 1) + m0(1, 2)*m1(2, 1);
	m4(1, 2) = m0(1, 0)*m1(0, 2) + m0(1, 1)*m1(1, 2) + m0(1, 2)*m1(2, 2);
	m4(2, 0) = m0(2, 0)*m1(0, 0) + m0(2, 1)*m1(1, 0) + m0(2, 2)*m1(2, 0);
	m4(2, 1) = m0(2, 0)*m1(0, 1) + m0(2, 1)*m1(1, 1) + m0(2, 2)*m1(2, 1);
	m4(2, 2) = m0(2, 0)*m1(0, 2) + m0(2, 1)*m1(1, 2) + m0(2, 2)*m1(2, 2);

	m4(0, 3) = m0(0, 0)*m1(0, 3) + m0(0, 1)*m1(1, 3) + m0(0, 2)*m1(2, 3) + m0(0, 3);
	m4(1, 3) = m0(1, 0)*m1(0, 3) + m0(1, 1)*m1(1, 3) + m0(1, 2)*m1(2, 3) + m0(1, 3);
	m4(2, 3) = m0(2, 0)*m1(0, 3) + m0(2, 1)*m1(1, 3) + m0(2, 2)*m1(2, 3) + m0(2, 3);

	m4(3,0) = m4(3,1) = m4(3,2) = 0.0;
	m4(3,3) = 1.0;

	return m4;
}


} // end namespace
