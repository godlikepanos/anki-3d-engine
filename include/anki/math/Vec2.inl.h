#include "anki/math/MathCommonSrc.h"


namespace anki {


//==============================================================================
// Constructors                                                                =
//==============================================================================

// default
inline Vec2::Vec2()
{
	x() = y() = 0.0;
}


// float
inline Vec2::Vec2(const float f)
{
	x() = y() = f;
}


// float, float
inline Vec2::Vec2(const float x_, const float y_)
{
	x() = x_;
	y() = y_;
}


// float[]
inline Vec2::Vec2(const float arr[])
{
	x() = arr[0];
	y() = arr[1];
}


// Copy
inline Vec2::Vec2(const Vec2& b)
{
	x() = b.x();
	y() = b.y();
}


// vec3
inline Vec2::Vec2(const Vec3& v3)
{
	x() = v3.x();
	y() = v3.y();
}


// vec4
inline Vec2::Vec2(const Vec4& v4)
{
	x() = v4.x();
	y() = v4.y();
}


//==============================================================================
// Accessors                                                                   =
//==============================================================================

inline float& Vec2::x()
{
	return vec.x;
}


inline float Vec2::x() const
{
	return vec.x;
}


inline float& Vec2::y()
{
	return vec.y;
}


inline float Vec2::y() const
{
	return vec.y;
}


inline float& Vec2::operator[](const size_t i)
{
	return arr[i];
}


inline float Vec2::operator[](const size_t i) const
{
	return arr[i];
}


//==============================================================================
// Operators with same                                                         =
//==============================================================================

// =
inline Vec2& Vec2::operator=(const Vec2& b)
{
	x() = b.x();
	y() = b.y();
	return *this;
}


// +
inline Vec2 Vec2::operator+(const Vec2& b) const
{
	return Vec2(x() + b.x(), y() + b.y());
}


// +=
inline Vec2& Vec2::operator+=(const Vec2& b)
{
	x() += b.x();
	y() += b.y();
	return (*this);
}


// -
inline Vec2 Vec2::operator-(const Vec2& b) const
{
	return Vec2(x() - b.x(), y() - b.y());
}


// -=
inline Vec2& Vec2::operator-=(const Vec2& b)
{
	x() -= b.x();
	y() -= b.y();
	return (*this);
}


// *
inline Vec2 Vec2::operator*(const Vec2& b) const
{
	return Vec2(x() * b.x(), y() * b.y());
}


// *=
inline Vec2& Vec2::operator*=(const Vec2& b)
{
	x() *= b.x();
	y() *= b.y();
	return (*this);
}


// /
inline Vec2 Vec2::operator/(const Vec2& b) const
{
	return Vec2(x() / b.x(), y() / b.y());
}


// /=
inline Vec2& Vec2::operator/=(const Vec2& b)
{
	x() /= b.x();
	y() /= b.y();
	return (*this);
}


// negative
inline Vec2 Vec2::operator-() const
{
	return Vec2(-x(), -y());
}


// ==
inline bool Vec2::operator==(const Vec2& b) const
{
	return Math::isZero(x() - b.x()) &&
		Math::isZero(y() - b.y());
}


// !=
inline bool Vec2::operator!=(const Vec2& b) const
{
	return !(*this == b);
}


//==============================================================================
// Operators with float                                                        =
//==============================================================================

// vec2 + float
inline Vec2 Vec2::operator+(float f) const
{
	return (*this) + Vec2(f);
}


// vec2 += float
inline Vec2& Vec2::operator+=(float f)
{
	(*this) += Vec2(f);
	return (*this);
}


// vec2 - float
inline Vec2 Vec2::operator-(float f) const
{
	return (*this) - Vec2(f);
}


// vec2 -= float
inline Vec2& Vec2::operator-=(float f)
{
	(*this) -= Vec2(f);
	return (*this);
}


// vec2 * float
inline Vec2 Vec2::operator*(float f) const
{
	return (*this) * Vec2(f);
}


// vec2 *= float
inline Vec2& Vec2::operator*=(float f)
{
	(*this) *= Vec2(f);
	return (*this);
}


// vec2 / float
inline Vec2 Vec2::operator/(float f) const
{
	return (*this) / Vec2(f);
}


// vec2 /= float
inline Vec2& Vec2::operator/=(float f)
{
	(*this) /= Vec2(f);
	return (*this);
}


//==============================================================================
// Misc methods                                                                =
//==============================================================================

// getLength
inline float Vec2::getLength() const
{
	return Math::sqrt(x() * x() + y() * y());
}


// normalize
inline void Vec2::normalize()
{
	(*this) /= getLength();
}


// Normalized (return the normalized)
inline Vec2 Vec2::getNormalized() const
{
	return (*this) / getLength();
}


// dot
inline float Vec2::dot(const Vec2& b) const
{
	return x() * b.x() + y() * b.y();
}


//==============================================================================
// Friends                                                                     =
//==============================================================================

// float + vec2
inline Vec2 operator+(float f, const Vec2& v2)
{
	return v2 + f;
}


// float - vec2
inline Vec2 operator-(float f, const Vec2& v2)
{
	return Vec2(f - v2.x(), f - v2.y());
}


// float * vec2
inline Vec2 operator*(float f, const Vec2& v2)
{
	return v2 * f;
}


// float / vec2
inline Vec2 operator/(float f, const Vec2& v2)
{
	return Vec2(f / v2.x(), f / v2.y());
}


// Print
inline std::ostream& operator<<(std::ostream& s, const Vec2& v)
{
	s << v.x() << ' ' << v.y();
	return s;
}


} // end namespace
