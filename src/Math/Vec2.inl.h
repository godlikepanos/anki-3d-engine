#include "MathCommon.inl.h"


#define ME (*this)


namespace M {


// accessors

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

inline float& Vec2::operator[](uint i)
{
	return arr[i];
}

inline float Vec2::operator[](uint i) const
{
	return arr[i];
}

// constructor []
inline Vec2::Vec2()
{
	x() = y() = 0.0;
}

// constructor [float, float]
inline Vec2::Vec2(float x_, float y_)
{
	x() = x_;
	y() = y_;
}

// constructor [float]
inline Vec2::Vec2(float f)
{
	x() = y() = f;
}

// constructor [float[]]
inline Vec2::Vec2(float arr[])
{
	x() = arr[0];
	y() = arr[1];
}

// constructor [vec2]
inline Vec2::Vec2(const Vec2& b)
{
	x() = b.x();
	y() = b.y();
}

// constructor [vec3]
inline Vec2::Vec2(const Vec3& v3)
{
	x() = v3.x();
	y() = v3.y();
}

// constructor [vec4]
inline Vec2::Vec2(const Vec4& v4)
{
	x() = v4.x();
	y() = v4.y();
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
	return ME;
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
	return ME;
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
	return ME;
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
	return ME;
}

// negative
inline Vec2 Vec2::operator-() const
{
	return Vec2(-x(), -y());
}

// ==
inline bool Vec2::operator==(const Vec2& b) const
{
	return isZero(x() - b.x()) && isZero(y() - b.y());
}

// !=
inline bool Vec2::operator!=(const Vec2& b) const
{
	return !(isZero(x() - b.x()) && isZero(y() - b.y()));
}

// vec2 + float
inline Vec2 Vec2::operator+(float f) const
{
	return ME + Vec2(f);
}

// float + vec2
inline Vec2 operator+(float f, const Vec2& v2)
{
	return v2 + f;
}

// vec2 += float
inline Vec2& Vec2::operator+=(float f)
{
	ME += Vec2(f);
	return ME;
}

// vec2 - float
inline Vec2 Vec2::operator-(float f) const
{
	return ME - Vec2(f);
}

// float - vec2
inline Vec2 operator-(float f, const Vec2& v2)
{
	return Vec2(f - v2.x(), f - v2.y());
}

// vec2 -= float
inline Vec2& Vec2::operator-=(float f)
{
	ME -= Vec2(f);
	return ME;
}

// vec2 * float
inline Vec2 Vec2::operator*(float f) const
{
	return ME * Vec2(f);
}

// float * vec2
inline Vec2 operator*(float f, const Vec2& v2)
{
	return v2 * f;
}

// vec2 *= float
inline Vec2& Vec2::operator*=(float f)
{
	ME *= Vec2(f);
	return ME;
}

// vec2 / float
inline Vec2 Vec2::operator/(float f) const
{
	return ME / Vec2(f);
}

// float / vec2
inline Vec2 operator/(float f, const Vec2& v2)
{
	return Vec2(f / v2.x(), f / v2.y());
}

// vec2 /= float
inline Vec2& Vec2::operator/=(float f)
{
	ME /= Vec2(f);
	return ME;
}

// getLength
inline float Vec2::getLength() const
{
	return M::sqrt(x() * x() + y() * y());
}

// normalize
inline void Vec2::normalize()
{
	ME /= getLength();
}

// Normalized (return the normalized)
inline Vec2 Vec2::getNormalized() const
{
	return ME / getLength();
}

// dot
inline float Vec2::dot(const Vec2& b) const
{
	return x() * b.x() + y() * b.y();
}

// print
inline std::ostream& operator<<(std::ostream& s, const Vec2& v)
{
	s << v.x() << ' ' << v.y();
	return s;
}


} // end namespace
