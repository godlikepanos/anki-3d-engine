#include "MathDfltHeader.h"


#define ME (*this)


namespace M {


// accessors
inline float& Vec4::operator [](uint i)
{
	return (&x)[i];
}

inline float Vec4::operator [](uint i) const
{
	return (&x)[i];
}

// constructor []
inline Vec4::Vec4()
	: x(0.0), y(0.0), z(0.0), w(0.0)
{}

// constructor [float]
inline Vec4::Vec4(float f)
	: x(f), y(f), z(f), w(f)
{}

// constructor [float, float, float, float]
inline Vec4::Vec4(float x_, float y_, float z_, float w_)
	: x(x_), y(y_), z(z_), w(w_)
{}

// constructor [vec2, float, float]
inline Vec4::Vec4(const Vec2& v2, float z_, float w_)
	: x(v2.x), y(v2.y), z(z_), w(w_)
{}

// constructor [vec3, float]
inline Vec4::Vec4(const Vec3& v3, float w_)
	: x(v3.x), y(v3.y), z(v3.z), w(w_)
{}

// constructor [vec4]
inline Vec4::Vec4(const Vec4& b)
	: x(b.x), y(b.y), z(b.z), w(b.w)
{}

// constructor [quat]
inline Vec4::Vec4(const Quat& q)
	: x(q.x), y(q.y), z(q.z), w(q.w)
{}

// +
inline Vec4 Vec4::operator +(const Vec4& b) const
{
	return Vec4(x+b.x, y+b.y, z+b.z, w+b.w);
}

// +=
inline Vec4& Vec4::operator +=(const Vec4& b)
{
	x += b.x;
	y += b.y;
	z += b.z;
	w += b.w;
	return ME;
}

// -
inline Vec4 Vec4::operator -(const Vec4& b) const
{
	return Vec4(x-b.x, y-b.y, z-b.z, w-b.w);
}

// -=
inline Vec4& Vec4::operator -=(const Vec4& b)
{
	x -= b.x;
	y -= b.y;
	z -= b.z;
	w -= b.w;
	return ME;
}

// *
inline Vec4 Vec4::operator *(const Vec4& b) const
{
	return Vec4(x*b.x, y*b.y, z*b.z, w*b.w);
}

// *=
inline Vec4& Vec4::operator *=(const Vec4& b)
{
	x *= b.x;
	y *= b.y;
	z *= b.z;
	w *= b.w;
	return ME;
}

// /
inline Vec4 Vec4::operator /(const Vec4& b) const
{
	return Vec4(x/b.x, y/b.y, z/b.z, w/b.w);
}

// /=
inline Vec4& Vec4::operator /=(const Vec4& b)
{
	x /= b.x;
	y /= b.y;
	z /= b.z;
	w /= b.w;
	return ME;
}

// negative
inline Vec4 Vec4::operator -() const
{
	return Vec4(-x, -y, -z, -w);
}

// ==
inline bool Vec4::operator ==(const Vec4& b) const
{
	return (isZero(x-b.x) && isZero(y-b.y) && isZero(z-b.z) && isZero(w-b.w)) ? true : false;
}

// !=
inline bool Vec4::operator !=(const Vec4& b) const
{
	return (isZero(x-b.x) && isZero(y-b.y) && isZero(z-b.z) && isZero(w-b.w)) ? false : true;
}

// vec4 + float
inline Vec4 Vec4::operator +(float f) const
{
	return ME + Vec4(f);
}

// float + vec4
inline Vec4 operator +(float f, const Vec4& v4)
{
	return v4+f;
}

// vec4 += float
inline Vec4& Vec4::operator +=(float f)
{
	ME += Vec4(f);
	return ME;
}

// vec4 - float
inline Vec4 Vec4::operator -(float f) const
{
	return ME - Vec4(f);
}

// float - vec4
inline Vec4 operator -(float f, const Vec4& v4)
{
	return Vec4(f-v4.x, f-v4.y, f-v4.z, f-v4.w);
}

// vec4 -= float
inline Vec4& Vec4::operator -=(float f)
{
	ME -= Vec4(f);
	return ME;
}

// vec4 * float
inline Vec4 Vec4::operator *(float f) const
{
	return ME * Vec4(f);
}

// float * vec4
inline Vec4 operator *(float f, const Vec4& v4)
{
	return v4*f;
}

// vec4 *= float
inline Vec4& Vec4::operator *=(float f)
{
	ME *= Vec4(f);
	return ME;
}

// vec4 / float
inline Vec4 Vec4::operator /(float f) const
{
	return ME / Vec4(f);
}

// float / vec4
inline Vec4 operator /(float f, const Vec4& v4)
{
	return Vec4(f/v4.x, f/v4.y, f/v4.z, f/v4.w);
}

// vec4 /= float
inline Vec4& Vec4::operator /=(float f)
{
	ME /= Vec4(f);
	return ME;
}

// vec4 * mat4
inline Vec4 Vec4::operator *(const Mat4& m4) const
{
	return Vec4(
		x*m4(0, 0) + y*m4(1, 0) + z*m4(2, 0) + w*m4(3, 0),
		x*m4(0, 1) + y*m4(1, 1) + z*m4(2, 1) + w*m4(3, 1),
		x*m4(0, 2) + y*m4(1, 2) + z*m4(2, 2) + w*m4(3, 2),
		x*m4(0, 3) + y*m4(1, 3) + z*m4(2, 3) + w*m4(3, 3)
	);
}

// dot
inline float Vec4::dot(const Vec4& b) const
{
	return x*b.x + y*b.y + z*b.z + w*b.w;
}

// getLength
inline float Vec4::getLength() const
{
	return M::sqrt(x*x + y*y + z*z + w*w);
}

// Normalized
inline Vec4 Vec4::getNormalized() const
{
	return ME * invSqrt(x*x +y*y + z*z + w*w);
}

// normalize
inline void Vec4::normalize()
{
	ME *= invSqrt(x*x +y*y + z*z + w*w);
}

// print
inline void Vec4::print() const
{
	for(int i=0; i<4; i++)
		cout << fixed << ME[i] << " ";
	cout << "\n" << endl;
}


} // end namespace
