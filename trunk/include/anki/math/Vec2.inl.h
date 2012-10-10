#include "anki/math/CommonSrc.h"

namespace anki {

//==============================================================================
// Constructors                                                                =
//==============================================================================

// default
inline Vec2::Vec2()
{
	x() = y() = 0.0;
}

// F32
inline Vec2::Vec2(const F32 f)
{
	x() = y() = f;
}

// F32, F32
inline Vec2::Vec2(const F32 x_, const F32 y_)
{
	x() = x_;
	y() = y_;
}

// F32[]
inline Vec2::Vec2(const F32 arr[])
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

inline F32& Vec2::x()
{
	return vec.x;
}

inline F32 Vec2::x() const
{
	return vec.x;
}

inline F32& Vec2::y()
{
	return vec.y;
}

inline F32 Vec2::y() const
{
	return vec.y;
}

inline F32& Vec2::operator[](const U i)
{
	return arr[i];
}

inline F32 Vec2::operator[](const U i) const
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
inline Bool Vec2::operator==(const Vec2& b) const
{
	return isZero(x() - b.x()) &&
		isZero(y() - b.y());
}

// !=
inline Bool Vec2::operator!=(const Vec2& b) const
{
	return !(*this == b);
}

// <
inline Bool Vec2::operator<(const Vec2& b) const
{
	return vec.x < b.vec.x && vec.y < b.vec.y;
}

// <=
inline Bool Vec2::operator<=(const Vec2& b) const
{
	return vec.x <= b.vec.x && vec.y <= b.vec.y;
}

//==============================================================================
// Operators with F32                                                          =
//==============================================================================

// vec2 + F32
inline Vec2 Vec2::operator+(F32 f) const
{
	return (*this) + Vec2(f);
}

// vec2 += F32
inline Vec2& Vec2::operator+=(F32 f)
{
	(*this) += Vec2(f);
	return (*this);
}

// vec2 - F32
inline Vec2 Vec2::operator-(F32 f) const
{
	return (*this) - Vec2(f);
}

// vec2 -= F32
inline Vec2& Vec2::operator-=(F32 f)
{
	(*this) -= Vec2(f);
	return (*this);
}

// vec2 * F32
inline Vec2 Vec2::operator*(F32 f) const
{
	return (*this) * Vec2(f);
}

// vec2 *= F32
inline Vec2& Vec2::operator*=(F32 f)
{
	(*this) *= Vec2(f);
	return (*this);
}

// vec2 / F32
inline Vec2 Vec2::operator/(F32 f) const
{
	return (*this) / Vec2(f);
}

// vec2 /= F32
inline Vec2& Vec2::operator/=(F32 f)
{
	(*this) /= Vec2(f);
	return (*this);
}

//==============================================================================
// Misc methods                                                                =
//==============================================================================

// getLengthSquared
inline F32 Vec2::getLengthSquared() const
{
	return x() * x() + y() * y();
}

// getLength
inline F32 Vec2::getLength() const
{
	return sqrt(getLengthSquared());
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
inline F32 Vec2::dot(const Vec2& b) const
{
	return x() * b.x() + y() * b.y();
}

//==============================================================================
// Friends                                                                     =
//==============================================================================

// F32 + vec2
inline Vec2 operator+(F32 f, const Vec2& v2)
{
	return v2 + f;
}

// F32 - vec2
inline Vec2 operator-(F32 f, const Vec2& v2)
{
	return Vec2(f - v2.x(), f - v2.y());
}

// F32 * vec2
inline Vec2 operator*(F32 f, const Vec2& v2)
{
	return v2 * f;
}

// F32 / vec2
inline Vec2 operator/(F32 f, const Vec2& v2)
{
	return Vec2(f / v2.x(), f / v2.y());
}

// Print
inline std::ostream& operator<<(std::ostream& s, const Vec2& v)
{
	s << v.x() << ' ' << v.y();
	return s;
}

} // end namespace anki
