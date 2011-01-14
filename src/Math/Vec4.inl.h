#include "MathCommon.inl.h"


#define SELF (*this)


namespace M {


// accessors
inline float& Vec4::operator[](uint i)
{
	return arr[i];
}

inline float Vec4::operator[](uint i) const
{
	return arr[i];
}

inline float& Vec4::x()
{
	return vec.x;
}

inline float Vec4::x() const
{
	return vec.x;
}

inline float& Vec4::y()
{
	return vec.y;
}

inline float Vec4::y() const
{
	return vec.y;
}

inline float& Vec4::z()
{
	return vec.z;
}

inline float Vec4::z() const
{
	return vec.z;
}

inline float& Vec4::w()
{
	return vec.w;
}

inline float Vec4::w() const
{
	return vec.w;
}

// default constructor
inline Vec4::Vec4()
{
	#if defined(MATH_INTEL_SIMD)
		mm = _mm_setzero_ps();
	#else
		x() = y() = z() = w() = 0.0;
	#endif
}

// constructor [float]
inline Vec4::Vec4(float f)
{
	#if defined(MATH_INTEL_SIMD)
		mm = _mm_set1_ps(f);
	#else
		x() = y() = z() = w() = f;
	#endif
}

// Constructor [float[]]
inline Vec4::Vec4(float arr_[])
{
	#if defined(MATH_INTEL_SIMD)
		mm = _mm_load_ps(arr_);
	#else
		arr[0] = arr_[0];
		arr[1] = arr_[1];
		arr[2] = arr_[2];
		arr[3] = arr_[3];
	#endif
}

// constructor [float, float, float, float]
inline Vec4::Vec4(float x_, float y_, float z_, float w_)
{
	#if defined(MATH_INTEL_SIMD)
		mm = _mm_set_ps(w_, z_, y_, x_);
	#else
		x() = x_;
		y() = y_;
		z() = z_;
		w() = w_;
	#endif
}

// constructor [__m128]
#if defined(MATH_INTEL_SIMD)
	inline Vec4::Vec4(const __m128& mm_)
	{
		mm = mm_;
	}
#endif

// constructor [vec2, float, float]
inline Vec4::Vec4(const Vec2& v2, float z_, float w_)
{
	x() = v2.x;
	y() = v2.y;
	z() = z_;
	w() = w_;
}

// constructor [vec3, float]
inline Vec4::Vec4(const Vec3& v3, float w_)
{
	x() = v3.x;
	y() = v3.y;
	z() = v3.z;
	w() = w_;
}

// constructor [vec4]
inline Vec4::Vec4(const Vec4& b)
{
	#if defined(MATH_INTEL_SIMD)
		mm = b.mm;
	#else
		x() = b.x();
		y() = b.y();
		z() = b.z();
		w() = b.w();
	#endif
}

// constructor [quat]
inline Vec4::Vec4(const Quat& q)
{
	x() = q.x;
	y() = q.y;
	z() = q.z;
	w() = q.w;
}

// +
inline Vec4 Vec4::operator+(const Vec4& b) const
{
	#if defined(MATH_INTEL_SIMD)
		return Vec4(_mm_add_ps(mm, b.mm));
	#else
		return Vec4(x() + b.x(), y() + b.y(), z() + b.z(), w() + b.w());
	#endif
}

// +=
inline Vec4& Vec4::operator+=(const Vec4& b)
{
	#if defined(MATH_INTEL_SIMD)
		mm = _mm_add_ps(mm, b.mm);
	#else
		x() += b.x();
		y() += b.y();
		z() += b.z();
		w() += b.w();
	#endif
	return SELF;
}

// -
inline Vec4 Vec4::operator-(const Vec4& b) const
{
	#if defined(MATH_INTEL_SIMD)
		return Vec4(_mm_sub_ps(mm, b.mm));
	#else
		return Vec4(x() - b.x(), y() - b.y(), z() - b.z(), w() - b.w());
	#endif
}

// -=
inline Vec4& Vec4::operator-=(const Vec4& b)
{
	#if defined(MATH_INTEL_SIMD)
		mm = _mm_sub_ps(mm, b.mm);
	#else
		x() -= b.x();
		y() -= b.y();
		z() -= b.z();
		w() -= b.w();
	#endif
	return SELF;
}

// *
inline Vec4 Vec4::operator*(const Vec4& b) const
{
	return Vec4(x() * b.x(), y() * b.y(), z() * b.z(), w() * b.w());
}

// *=
inline Vec4& Vec4::operator*=(const Vec4& b)
{
	x() *= b.x();
	y() *= b.y();
	z() *= b.z();
	w() *= b.w();
	return SELF;
}

// /
inline Vec4 Vec4::operator/(const Vec4& b) const
{
	return Vec4(x() / b.x(), y() / b.y(), z() / b.z(), w() / b.w());
}

// /=
inline Vec4& Vec4::operator/=(const Vec4& b)
{
	x() /= b.x();
	y() /= b.y();
	z() /= b.z();
	w() /= b.w();
	return SELF;
}

// negative
inline Vec4 Vec4::operator-() const
{
	return Vec4(-x(), -y(), -z(), -w());
}

// ==
inline bool Vec4::operator==(const Vec4& b) const
{
	return isZero(x() - b.x()) && isZero(y() - b.y()) && isZero(z() - b.z()) && isZero(w() - b.w());
}

// !=
inline bool Vec4::operator!=(const Vec4& b) const
{
	return isZero(x() - b.x()) && isZero(y() - b.y()) && isZero(z() - b.z()) && isZero(w() - b.w());
}

// vec4 + float
inline Vec4 Vec4::operator+(float f) const
{
	return SELF + Vec4(f);
}

// float + vec4
inline Vec4 operator+(float f, const Vec4& v4)
{
	return v4 + f;
}

// vec4 += float
inline Vec4& Vec4::operator+=(float f)
{
	SELF += Vec4(f);
	return SELF;
}

// vec4 - float
inline Vec4 Vec4::operator-(float f) const
{
	return SELF - Vec4(f);
}

// float - vec4
inline Vec4 operator-(float f, const Vec4& v4)
{
	return Vec4(f - v4.x(), f - v4.y(), f - v4.z(), f -v4.w());
}

// vec4 -= float
inline Vec4& Vec4::operator-=(float f)
{
	SELF -= Vec4(f);
	return SELF;
}

// vec4 * float
inline Vec4 Vec4::operator*(float f) const
{
	return SELF * Vec4(f);
}

// float * vec4
inline Vec4 operator*(float f, const Vec4& v4)
{
	return v4 * f;
}

// vec4 *= float
inline Vec4& Vec4::operator*=(float f)
{
	SELF *= Vec4(f);
	return SELF;
}

// vec4 / float
inline Vec4 Vec4::operator/(float f) const
{
	return SELF / Vec4(f);
}

// float / vec4
inline Vec4 operator/(float f, const Vec4& v4)
{
	return Vec4(f / v4.x(), f / v4.y(), f / v4.z(), f / v4.w());
}

// vec4 /= float
inline Vec4& Vec4::operator/=(float f)
{
	SELF /= Vec4(f);
	return SELF;
}

// vec4 * mat4
inline Vec4 Vec4::operator*(const Mat4& m4) const
{
	return Vec4(
		x() * m4(0, 0) + y() * m4(1, 0) + z() * m4(2, 0) + w() * m4(3, 0),
		x() * m4(0, 1) + y() * m4(1, 1) + z() * m4(2, 1) + w() * m4(3, 1),
		x() * m4(0, 2) + y() * m4(1, 2) + z() * m4(2, 2) + w() * m4(3, 2),
		x() * m4(0, 3) + y() * m4(1, 3) + z() * m4(2, 3) + w() * m4(3, 3)
	);
}

// dot
inline float Vec4::dot(const Vec4& b) const
{
	return x() * b.x() + y() * b.y() + z() * b.z() + w() * b.w();
}

// getLength
inline float Vec4::getLength() const
{
	return M::sqrt(x() * x() + y() * y() + z() * z() + w() * w());
}

// Normalized
inline Vec4 Vec4::getNormalized() const
{
	return SELF * invSqrt(x() * x() + y() * y() + z() * z() + w() * w());
}

// normalize
inline void Vec4::normalize()
{
	SELF *= invSqrt(x() * x() + y() * y() + z() * z() + w() * w());
}

// print
inline std::ostream& operator<<(std::ostream& s, const Vec4& v)
{
	s << v.x() << ' ' << v.y() << ' ' << v.z() << ' ' << v.w();
	return s;
}


} // end namespace
