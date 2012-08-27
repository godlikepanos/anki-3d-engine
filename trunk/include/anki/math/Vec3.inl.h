#include "anki/math/MathCommonSrc.h"

namespace anki {

//==============================================================================
// Constructors                                                                =
//==============================================================================

// Default
inline Vec3::Vec3()
{
	arr[0] = arr[1] = arr[2] = 0.0;
}

// F32, F32, F32
inline Vec3::Vec3(const F32 x_, const F32 y_, const F32 z_)
{
	x() = x_;
	y() = y_;
	z() = z_;
}

// F32
inline Vec3::Vec3(const F32 f)
{
	arr[0] = arr[1] = arr[2] = f;
}

// F32[]
inline Vec3::Vec3(const F32 arr_[])
{
	arr[0] = arr_[0];
	arr[1] = arr_[1];
	arr[2] = arr_[2];
}

// Copy
inline Vec3::Vec3(const Vec3& b)
{
	arr[0] = b.arr[0];
	arr[1] = b.arr[1];
	arr[2] = b.arr[2];
}

// Vec2, F32
inline Vec3::Vec3(const Vec2& v2, const F32 z_)
{
	x() = v2.x();
	y() = v2.y();
	z() = z_;
}

// Vec4
inline Vec3::Vec3(const Vec4& v4)
{
	arr[0] = v4[0];
	arr[1] = v4[1];
	arr[2] = v4[2];
}

// Quat
inline Vec3::Vec3(const Quat& q)
{
	x() = q.x();
	y() = q.y();
	z() = q.z();
}

//==============================================================================
// Accessors                                                                   =
//==============================================================================

inline F32& Vec3::x()
{
	return vec.x;
}

inline F32 Vec3::x() const
{
	return vec.x;
}

inline F32& Vec3::y()
{
	return vec.y;
}

inline F32 Vec3::y() const
{
	return vec.y;
}

inline F32& Vec3::z()
{
	return vec.z;
}

inline F32 Vec3::z() const
{
	return vec.z;
}

inline F32& Vec3::operator[](const U i)
{
	return arr[i];
}

inline F32 Vec3::operator[](const U i) const
{
	return arr[i];
}

inline Vec2 Vec3::xy() const
{
	return Vec2(x(), y());
}

//==============================================================================
// Operators with same type                                                    =
//==============================================================================

// =
inline Vec3& Vec3::operator=(const Vec3& b)
{
	arr[0] = b.arr[0];
	arr[1] = b.arr[1];
	arr[2] = b.arr[2];
	return (*this);
}

// +
inline Vec3 Vec3::operator+(const Vec3& b) const
{
	return Vec3(x() + b.x(), y() + b.y(), z() + b.z());
}

// +=
inline Vec3& Vec3::operator+=(const Vec3& b)
{
	x() += b.x();
	y() += b.y();
	z() += b.z();
	return (*this);
}

// -
inline Vec3 Vec3::operator-(const Vec3& b) const
{
	return Vec3(x() - b.x(), y() - b.y(), z() - b.z());
}

// -=
inline Vec3& Vec3::operator-=(const Vec3& b)
{
	x() -= b.x();
	y() -= b.y();
	z() -= b.z();
	return (*this);
}

// *
inline Vec3 Vec3::operator*(const Vec3& b) const
{
	return Vec3(x() * b.x(), y() * b.y(), z() * b.z());
}

// *=
inline Vec3& Vec3::operator*=(const Vec3& b)
{
	x() *= b.x();
	y() *= b.y();
	z() *= b.z();
	return (*this);
}

// /
inline Vec3 Vec3::operator/(const Vec3& b) const
{
	return Vec3(x() / b.x(), y() / b.y(), z() / b.z());
}

// /=
inline Vec3& Vec3::operator/=(const Vec3& b)
{
	x() /= b.x();
	y() /= b.y();
	z() /= b.z();
	return (*this);
}

// negative
inline Vec3 Vec3::operator-() const
{
	return Vec3(-x(), -y(), -z());
}

// ==
inline Bool Vec3::operator==(const Vec3& b) const
{
	return Math::isZero(x() - b.x()) 
		&& Math::isZero(y() - b.y()) 
		&& Math::isZero(z() - b.z());
}

// !=
inline Bool Vec3::operator!=(const Vec3& b) const
{
	return !operator==(b);
}

// <
inline Bool Vec3::operator<(const Vec3& b) const
{
	return x() < b.x() && y() < b.y() && z() < b.z();
}

// <=
inline Bool Vec3::operator<=(const Vec3& b) const
{
	return x() <= b.x() && y() <= b.y() && z() <= b.z();
}

// >
inline Bool Vec3::operator>(const Vec3& b) const
{
	return x() > b.x() && y() > b.y() && z() > b.z();
}

// >=
inline Bool Vec3::operator>=(const Vec3& b) const
{
	return x() >= b.x() && y() >= b.y() && z() >= b.z();
}

//==============================================================================
// Operators with F32                                                        =
//==============================================================================

// Vec3 + F32
inline Vec3 Vec3::operator+(F32 f) const
{
	return (*this) + Vec3(f);
}

// Vec3 += F32
inline Vec3& Vec3::operator+=(F32 f)
{
	(*this) += Vec3(f);
	return (*this);
}

// Vec3 - F32
inline Vec3 Vec3::operator-(F32 f) const
{
	return (*this) - Vec3(f);
}

// Vec3 -= F32
inline Vec3& Vec3::operator-=(F32 f)
{
	(*this) -= Vec3(f);
	return (*this);
}

// Vec3 * F32
inline Vec3 Vec3::operator*(F32 f) const
{
	return (*this) * Vec3(f);
}

// Vec3 *= F32
inline Vec3& Vec3::operator*=(F32 f)
{
	(*this) *= Vec3(f);
	return (*this);
}

// Vec3 / F32
inline Vec3 Vec3::operator/(F32 f) const
{
	return (*this) / Vec3(f);
}

// Vec3 /= F32
inline Vec3& Vec3::operator/=(F32 f)
{
	(*this) /= Vec3(f);
	return (*this);
}

//==============================================================================
// Other                                                                       =
//==============================================================================

// dot
inline F32 Vec3::dot(const Vec3& b) const
{
	return x() * b.x() + y() * b.y() + z() * b.z();
}

// cross prod
inline Vec3 Vec3::cross(const Vec3& b) const
{
	return Vec3(y() * b.z() - z() * b.y(),
		z() * b.x() - x() * b.z(),
		x() * b.y() - y() * b.x());
}

// getLength
inline F32 Vec3::getLength() const
{
	return Math::sqrt(getLengthSquared());
}

// getLengthSquared
inline F32 Vec3::getLengthSquared() const
{
	return x() * x() + y() * y() + z() * z();
}

// getDistanceSquared
inline F32 Vec3::getDistanceSquared(const Vec3& b) const
{
	return ((*this) - b).getLengthSquared();
}

// normalize
inline void Vec3::normalize()
{
	(*this) /= getLength();
}

// getNormalized
inline Vec3 Vec3::getNormalized() const
{
	return (*this) / getLength();
}

// getProjection
inline Vec3 Vec3::getProjection(const Vec3& toThis) const
{
	return toThis * ((*this).dot(toThis) / (toThis.dot(toThis)));
}

// getRotated
inline Vec3 Vec3::getRotated(const Quat& q) const
{
	ANKI_ASSERT(Math::isZero(1.0 - q.getLength())); // Not normalized quat
	Vec3 qXyz(q);
	return (*this) + qXyz.cross(qXyz.cross((*this)) + (*this) * q.w()) * 2.0;
}

// rotate
inline void Vec3::rotate(const Quat& q)
{
	(*this) = getRotated(q);
}

// lerp
inline Vec3 Vec3::lerp(const Vec3& v1, F32 t) const
{
	return ((*this) * (1.0 - t)) + (v1 * t);
}

//==============================================================================
// Transformations                                                             =
//==============================================================================

// Mat3
inline Vec3 Vec3::getTransformed(const Vec3& translate, const Mat3& rotate,
	F32 scale) const
{
	return (rotate * ((*this) * scale)) + translate;
}

// Mat3
inline void Vec3::transform(const Vec3& translate, const Mat3& rotate,
	F32 scale)
{
	(*this) = getTransformed(translate, rotate, scale);
}

// Mat3 no scale
inline Vec3 Vec3::getTransformed(const Vec3& translate,
	const Mat3& rotate) const
{
	return (rotate * (*this)) + translate;
}

// Mat3 no scale
inline void Vec3::transform(const Vec3& translate, const Mat3& rotate)
{
	(*this) = getTransformed(translate, rotate);
}

// Quat
inline Vec3 Vec3::getTransformed(const Vec3& translate, const Quat& rotate,
	F32 scale) const
{
	return ((*this) * scale).getRotated(rotate) + translate;
}

// Quat
inline void Vec3::transform(const Vec3& translate, const Quat& rotate,
	F32 scale)
{
	(*this) = getTransformed(translate, rotate, scale);
}

// Mat4
inline Vec3 Vec3::getTransformed(const Mat4& transform) const
{
#if defined(ANKI_MATH_INTEL_SIMD)
	Vec3 out;
	Vec4 v4((*this), 1.0);
	for(U i = 0; i < 3; i++)
	{
		_mm_store_ss(&out[i], _mm_dp_ps(transform.getMm(i), v4.getMm(), 0xF1));
	}
	return out;
#else
	return Vec3(transform(0, 0) * x() + transform(0, 1) * y() 
		+ transform(0, 2) * z() + transform(0, 3),
		transform(1, 0) * x() + transform(1, 1) * y() 
		+ transform(1, 2) * z() + transform(1, 3),
		transform(2, 0) * x() + transform(2, 1) * y() 
		+ transform(2, 2) * z() + transform(2, 3)
	);
#endif
}

// Mat4
inline void Vec3::transform(const Mat4& transform)
{
	(*this) = getTransformed(transform);
}

// Transform
inline Vec3 Vec3::getTransformed(const Transform& transform) const
{
	return (transform.getRotation() * ((*this) * transform.getScale())) 
		+ transform.getOrigin();
}

// Transform
inline void Vec3::transform(const Transform& transform)
{
	(*this) = getTransformed(transform);
}

//==============================================================================
// Friends                                                                     =
//==============================================================================

// F32 + Vec3
inline Vec3 operator+(const F32 f, const Vec3& v)
{
	return v + f;
}

// F32 - Vec3
inline Vec3 operator-(const F32 f, const Vec3& v)
{
	return Vec3(f - v.x(), f - v.y(), f - v.z());
}

// F32 * Vec3
inline Vec3 operator*(const F32 f, const Vec3& v)
{
	return v * f;
}

// F32 / Vec3
inline Vec3 operator/(const F32 f, const Vec3& v)
{
	return Vec3(f / v.x(), f / v.y(), f / v.z());
}

// Print
inline std::ostream& operator<<(std::ostream& s, const Vec3& v)
{
	s << v.x() << ' ' << v.y() << ' ' << v.z();
	return s;
}

} // end namespace
