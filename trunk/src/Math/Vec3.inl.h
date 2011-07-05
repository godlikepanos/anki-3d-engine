#include "Common.inl.h"


namespace M {


//==============================================================================
// Constructors                                                                =
//==============================================================================

// Default
inline Vec3::Vec3()
{
	arr[0] = arr[1] = arr[2] = 0.0;
}

// float, float, float
inline Vec3::Vec3(float x_, float y_, float z_)
{
	x() = x_;
	y() = y_;
	z() = z_;
}

// float
inline Vec3::Vec3(float f)
{
	arr[0] = arr[1] = arr[2] = f;
}

// float[]
inline Vec3::Vec3(float arr_[])
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

// Vec2, float
inline Vec3::Vec3(const Vec2& v2, float z_)
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
// Accessors                                                                            =
//==============================================================================

inline float& Vec3::x()
{
	return vec.x;
}

inline float Vec3::x() const
{
	return vec.x;
}

inline float& Vec3::y()
{
	return vec.y;
}

inline float Vec3::y() const
{
	return vec.y;
}

inline float& Vec3::z()
{
	return vec.z;
}

inline float Vec3::z() const
{
	return vec.z;
}

inline float& Vec3::operator[](uint i)
{
	return arr[i];
}

inline float Vec3::operator[](uint i) const
{
	return arr[i];
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
inline bool Vec3::operator==(const Vec3& b) const
{
	return isZero(x() - b.x()) && isZero(y() - b.y()) && isZero(z() - b.z());
}

// !=
inline bool Vec3::operator!=(const Vec3& b) const
{
	return !(isZero(x() - b.x()) && isZero(y() - b.y()) && isZero(z() - b.z()));
}


//==============================================================================
// Operators with float                                                        =
//==============================================================================

// Vec3 + float
inline Vec3 Vec3::operator+(float f) const
{
	return (*this) + Vec3(f);
}

// float + Vec3
inline Vec3 operator+(float f, const Vec3& v)
{
	return v + f;
}

// Vec3 += float
inline Vec3& Vec3::operator+=(float f)
{
	(*this) += Vec3(f);
	return (*this);
}

// Vec3 - float
inline Vec3 Vec3::operator-(float f) const
{
	return (*this) - Vec3(f);
}

// float - Vec3
inline Vec3 operator-(float f, const Vec3& v)
{
	return Vec3(f - v.x(), f - v.y(), f - v.z());
}

// Vec3 -= float
inline Vec3& Vec3::operator-=(float f)
{
	(*this) -= Vec3(f);
	return (*this);
}

// Vec3 * float
inline Vec3 Vec3::operator*(float f) const
{
	return (*this) * Vec3(f);
}

// float * Vec3
inline Vec3 operator*(float f, const Vec3& v)
{
	return v * f;
}

// Vec3 *= float
inline Vec3& Vec3::operator*=(float f)
{
	(*this) *= Vec3(f);
	return (*this);
}

// Vec3 / float
inline Vec3 Vec3::operator/(float f) const
{
	return (*this) / Vec3(f);
}

// float / Vec3
inline Vec3 operator/(float f, const Vec3& v)
{
	return Vec3(f / v.x(), f / v.y(), f / v.z());
}

// Vec3 /= float
inline Vec3& Vec3::operator/=(float f)
{
	(*this) /= Vec3(f);
	return (*this);
}


//==============================================================================
// Misc methods                                                                =
//==============================================================================

// dot
inline float Vec3::dot(const Vec3& b) const
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
inline float Vec3::getLength() const
{
	return M::sqrt(getLengthSquared());
}

// getLengthSquared
inline float Vec3::getLengthSquared() const
{
	return x() * x() + y() * y() + z() * z();
}

// getDistanceSquared
inline float Vec3::getDistanceSquared(const Vec3& b) const
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
	ASSERT(isZero(1.0 - q.getLength())); // Not normalized quat

	/*float vmult = 2.0f*(q.x*x + q.y*y + q.z*z);
	float crossmult = 2.0*q.w;
	float pmult = crossmult*q.w - 1.0;

	return Vec3(pmult*x + vmult*q.x + crossmult*(q.y*z - q.z*y),
							   pmult*y + vmult*q.y + crossmult*(q.z*x - q.x*z),
	               pmult*z + vmult*q.z + crossmult*(q.x*y - q.y*x));*/
	Vec3 qXyz(q);
	return (*this) + qXyz.cross(qXyz.cross((*this)) + (*this) * q.w()) * 2.0;
}

// rotate
inline void Vec3::rotate(const Quat& q)
{
	(*this) = getRotated(q);
}

// lerp
inline Vec3 Vec3::lerp(const Vec3& v1, float t) const
{
	return ((*this) * (1.0 - t)) + (v1 * t);
}


//==============================================================================
// Transformations                                                             =
//==============================================================================

// Mat3
inline Vec3 Vec3::getTransformed(const Vec3& translate, const Mat3& rotate,
	float scale) const
{
	return (rotate * ((*this) * scale)) + translate;
}

// Mat3
inline void Vec3::transform(const Vec3& translate, const Mat3& rotate,
	float scale)
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
	float scale) const
{
	return ((*this) * scale).getRotated(rotate) + translate;
}

// Quat
inline void Vec3::transform(const Vec3& translate, const Quat& rotate,
	float scale)
{
	(*this) = getTransformed(translate, rotate, scale);
}

// Mat4
inline Vec3 Vec3::getTransformed(const Mat4& transform) const
{
	#if defined(MATH_INTEL_SIMD)
		Vec3 out;
		Vec4 v4((*this), 1.0);
		for(int i = 0; i < 3; i++)
		{
			_mm_store_ss(&out[i], _mm_dp_ps(transform.getMm(i), v4.getMm(),
				0xF1));
		}
		return out;
	#else
		return Vec3(transform(0, 0) * x() + transform(0, 1) * y() +
			transform(0, 2) * z() + transform(0, 3),
			transform(1, 0) * x() + transform(1, 1) * y() +
			transform(1, 2) * z() + transform(1, 3),
			transform(2, 0) * x() + transform(2, 1) * y() +
			transform(2, 2) * z() + transform(2, 3)
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
	return (transform.getRotation() * ((*this) * transform.getScale())) +
		transform.getOrigin();
}

// Transform
inline void Vec3::transform(const Transform& transform)
{
	(*this) = getTransformed(transform);
}

//==============================================================================
// Print                                                                       =
//==============================================================================
inline std::ostream& operator<<(std::ostream& s, const Vec3& v)
{
	s << v.x() << ' ' << v.y() << ' ' << v.z();
	return s;
}


} // end namespace
