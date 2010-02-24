#include "m_dflt_header.h"


#define ME (*this)


namespace m {

// accessors
inline float& mat4_t::operator ()( const uint i, const uint j )
{
	return arr2[i][j];
}

inline const float& mat4_t::operator ()( const uint i, const uint j ) const
{
	return arr2[i][j];
}

inline float& mat4_t::operator []( const uint i) 
{ 
	return arr1[i]; 
}

inline const float& mat4_t::operator []( const uint i) const 
{ 
	return arr1[i]; 
}

// constructor [mat4]
inline mat4_t::mat4_t( const mat4_t& b )
{
	for( int i=0; i<16; i++ )
		ME[i] = b[i];
}

// constructor [float[]]
inline mat4_t::mat4_t( const float arr_ [] )
{
	for( int i=0; i<16; i++ )
		ME[i] = arr_[i];
}

// constructor [float..........]
inline mat4_t::mat4_t( float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33 )
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
inline mat4_t::mat4_t( const mat3_t& m3 )
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
inline mat4_t::mat4_t( const vec3_t& v )
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
inline mat4_t::mat4_t( const vec4_t& v )
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
inline mat4_t::mat4_t( const vec3_t& transl, const mat3_t& rot )
{
	SetRotationPart(rot);
	SetTranslationPart(transl);
	ME(3,0) = ME(3,1) = ME(3,2) = 0.0;
	ME(3,3) = 1.0;
}

// constructor [vec3, mat3, float]
inline mat4_t::mat4_t( const vec3_t& translate, const mat3_t& rotate, float scale )
{
	if( !IsZero( scale-1.0 ) )
		SetRotationPart( rotate*scale );
	else
		SetRotationPart( rotate );

	SetTranslationPart( translate );

	ME(3,0) = ME(3,1) = ME(3,2) = 0.0;
	ME(3,3) = 1.0;
}

// constructor [float]
inline mat4_t::mat4_t( float f )
{
	for( int i=0; i<16; i++ )
		ME[i] = f;
}

// 4x4 + 4x4
inline mat4_t mat4_t::operator +( const mat4_t& b ) const
{
	mat4_t c;
	for( int i=0; i<16; i++ )
		c[i] = ME[i] + b[i];
	return c;
}

// 4x4 + 4x4 (self)
inline mat4_t& mat4_t::operator +=( const mat4_t& b )
{
	for( int i=0; i<16; i++ )
		ME[i] += b[i];
	return ME;
}

// 4x4 - 4x4
inline mat4_t mat4_t::operator -( const mat4_t& b ) const
{
	mat4_t c;
	for( int i=0; i<16; i++ )
		c[i] = ME[i] - b[i];
	return c;
}

// 4x4 - 4x4 (self)
inline mat4_t& mat4_t::operator -=( const mat4_t& b )
{
	for( int i=0; i<16; i++ )
		ME[i] -= b[i];
	return ME;
}

// 4x4 * 4x4
inline mat4_t mat4_t::operator *( const mat4_t& b ) const
{
	mat4_t c;
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
inline mat4_t& mat4_t::operator *=( const mat4_t& b )
{
	ME = ME * b;
	return ME;
}

// ==
inline bool mat4_t::operator ==( const mat4_t& b ) const
{
	for( int i=0; i<16; i++ )
		if( !IsZero( ME[i]-b[i] ) ) return false;
	return true;
}

// !=
inline bool mat4_t::operator !=( const mat4_t& b ) const
{
	for( int i=0; i<16; i++ )
		if( !IsZero( ME[i]-b[i] ) ) return true;
	return false;
}

// 4x4 * vec4
inline vec4_t mat4_t::operator *( const vec4_t& b ) const
{
	return vec4_t(
		ME(0,0)*b.x + ME(0,1)*b.y + ME(0,2)*b.z + ME(0,3)*b.w,
		ME(1,0)*b.x + ME(1,1)*b.y + ME(1,2)*b.z + ME(1,3)*b.w,
		ME(2,0)*b.x + ME(2,1)*b.y + ME(2,2)*b.z + ME(2,3)*b.w,
		ME(3,0)*b.x + ME(3,1)*b.y + ME(3,2)*b.z + ME(3,3)*b.w
	);
}

// 4x4 + float
inline mat4_t mat4_t::operator +( float f ) const
{
	mat4_t c;
	for( int i=0; i<16; i++ )
		c[i] = ME[i] + f;
	return c;
}

// float + 4x4
inline mat4_t operator +( float f, const mat4_t& m4 )
{
	return m4+f;
}

// 4x4 + float (self)
inline mat4_t& mat4_t::operator +=( float f )
{
	for( int i=0; i<16; i++ )
		ME[i] += f;
	return ME;
}

// 4x4 - float
inline mat4_t mat4_t::operator -( float f ) const
{
	mat4_t c;
	for( int i=0; i<16; i++ )
		c[i] = ME[i] - f;
	return c;
}

// float - 4x4
inline mat4_t operator -( float f, const mat4_t& m4 )
{
	mat4_t out;
	for( int i=0; i<16; i++ )
		out[i] = f- m4[i];
	return out;
}

// 4x4 - float (self)
inline mat4_t& mat4_t::operator -=( float f )
{
	for( int i=0; i<16; i++ )
		ME[i] -= f;
	return ME;
}

// 4x4 * float
inline mat4_t mat4_t::operator *( float f ) const
{
	mat4_t c;
	for( int i=0; i<16; i++ )
		c[i] = ME[i] * f;
	return c;
}

// float * 4x4
inline mat4_t operator *( float f, const mat4_t& m4 )
{
	return m4*f;
}

// 4x4 *= float
inline mat4_t& mat4_t::operator *=( float f )
{
	for( int i=0; i<16; i++ )
		ME[i] *= f;
	return ME;
}

// 4x4 / float
inline mat4_t mat4_t::operator /( float f ) const
{
	mat4_t c;
	for( int i=0; i<16; i++ )
		c[i] = ME[i] / f;
	return c;
}

// float / 4x4
inline mat4_t operator /( float f, const mat4_t& m4 )
{
	mat4_t out;
	for( uint i=0; i<9; i++ )
		out[i] = f / m4[i];
	return out;
}

// 4x4 /= float
inline mat4_t& mat4_t::operator /=( float f )
{
	for( int i=0; i<16; i++ )
		ME[i] /= f;
	return ME;
}

// SetRows
inline void mat4_t::SetRows( const vec4_t& a, const vec4_t& b, const vec4_t& c, const vec4_t& d )
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

// SetRow
inline void mat4_t::SetRow( uint i, const vec4_t& v )
{
	DEBUG_ERR( i > 3 );
	ME(i,0) = v.x;
	ME(i,1) = v.y;
	ME(i,2) = v.z;
	ME(i,3) = v.w;
}

// SetColumns
inline void mat4_t::SetColumns( const vec4_t& a, const vec4_t& b, const vec4_t& c, const vec4_t& d )
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

// SetColumn
inline void mat4_t::SetColumn( uint i, const vec4_t& v )
{
	DEBUG_ERR( i > 3 );
	ME(0,i) = v.x;
	ME(1,i) = v.y;
	ME(2,i) = v.z;
	ME(3,i) = v.w;
}

// Transpose
inline void mat4_t::Transpose()
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

// GetTransposed
// return the transposed
inline mat4_t mat4_t::GetTransposed() const
{
	mat4_t m4;
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

// SetRotationPart
inline void mat4_t::SetRotationPart( const mat3_t& m3 )
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

// GetRotationPart
inline mat3_t mat4_t::GetRotationPart() const
{
	mat3_t m3;
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

// SetTranslationPart
inline void mat4_t::SetTranslationPart( const vec4_t& v )
{
	ME(0, 3) = v.x;
	ME(1, 3) = v.y;
	ME(2, 3) = v.z;
	ME(3, 3) = v.w;
}

// SetTranslationPart
inline void mat4_t::SetTranslationPart( const vec3_t& v )
{
	ME(0, 3) = v.x;
	ME(1, 3) = v.y;
	ME(2, 3) = v.z;
}

// GetTranslationPart
inline vec3_t mat4_t::GetTranslationPart() const
{
	return vec3_t( ME(0, 3), ME(1, 3), ME(2, 3) );
}

// GetIdentity
inline const mat4_t& mat4_t::GetIdentity()
{
	static mat4_t ident( 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0 );
	return ident;
}

// GetZero
inline const mat4_t& mat4_t::GetZero()
{
	static mat4_t zero( 0.0 );
	return zero;
}

// print
inline void mat4_t::print() const
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
inline float mat4_t::Det() const
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

// Invert
inline void mat4_t::Invert()
{
	ME = GetInverse();
}

// Inverted
inline mat4_t mat4_t::GetInverse() const
{
	float tmp[12];
	float det;
	const mat4_t& in = ME;

	mat4_t m4;


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

	DEBUG_ERR( IsZero( det ) ); // Cannot invert, det == 0
	det = 1/det;
	m4 *= det;
	return m4;
}


// GetInverseTransformation
inline mat4_t mat4_t::GetInverseTransformation() const
{
	mat3_t inverted_rot = (GetRotationPart()).GetTransposed();
	vec3_t inverted_tsl = GetTranslationPart();
	inverted_tsl = -( inverted_rot * inverted_tsl );
	return mat4_t( inverted_tsl, inverted_rot );
}

// Lerp
inline mat4_t mat4_t::Lerp( const mat4_t& b, float t ) const
{
	return (ME*(1.0-t))+(b*t);
}

// CombineTransformations
inline mat4_t mat4_t::CombineTransformations( const mat4_t& m0, const mat4_t& m1 )
{
	/* the clean code is:
	mat3_t rot = m0.GetRotationPart() * m1.GetRotationPart();  // combine the rotations
	vec3_t tra = (m1.GetTranslationPart()).Transformed( m0.GetTranslationPart(), m0.GetRotationPart(), 1.0 );
	return mat4_t( tra, rot );
	and the optimized: */
	DEBUG_ERR( !IsZero( m0(3,0)+m0(3,1)+m0(3,2)+m0(3,3)-1.0 ) ||
	           !IsZero( m1(3,0)+m1(3,1)+m1(3,2)+m1(3,3)-1.0 ) ); // one of the 2 mat4 doesnt represent transformation

	mat4_t m4;

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
