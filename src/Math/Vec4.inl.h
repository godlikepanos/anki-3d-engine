#include "MathCommon.inl.h"


#define SELF (*this)


namespace M {


//======================================================================================================================
// Constructors                                                                                                        =
//======================================================================================================================

// default
inline Vec4::Vec4()
{
	#if defined(MATH_INTEL_SIMD)
		mm = _mm_setzero_ps();
	#else
		arr[0] = arr[1] = arr[2] = arr[3] = 0.0;
	#endif
}

// float
inline Vec4::Vec4(float f)
{
	#if defined(MATH_INTEL_SIMD)
		mm = _mm_set1_ps(f);
	#else
		arr[0] = arr[1] = arr[2] = arr[3] = f;
	#endif
}

// float[]
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

// float, float, float, float
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

// vec2, float, float
inline Vec4::Vec4(const Vec2& v2, float z_, float w_)
{
	x() = v2.x();
	y() = v2.y();
	z() = z_;
	w() = w_;
}

// vec3, float
inline Vec4::Vec4(const Vec3& v3, float w_)
{
	x() = v3.x();
	y() = v3.y();
	z() = v3.z();
	w() = w_;
}

// copy
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

// quat
inline Vec4::Vec4(const Quat& q)
{
	x() = q.x;
	y() = q.y;
	z() = q.z;
	w() = q.w;
}

// __m128
#if defined(MATH_INTEL_SIMD)
	inline Vec4::Vec4(const __m128& mm_)
	{
		mm = mm_;
	}
#endif


//======================================================================================================================
// Accessors                                                                                                           =
//======================================================================================================================

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

#if defined(MATH_INTEL_SIMD)
	inline __m128& Vec4::getMm()
	{
		return mm;
	}

	inline const __m128& Vec4::getMm() const
	{
		return mm;
	}
#endif


//======================================================================================================================
// Operators with same                                                                                                 =
//======================================================================================================================

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
	#if defined(MATH_INTEL_SIMD)
		return Vec4(_mm_mul_ps(mm, b.mm));
	#else
		return Vec4(x() * b.x(), y() * b.y(), z() * b.z(), w() * b.w());
	#endif
}

// *=
inline Vec4& Vec4::operator*=(const Vec4& b)
{
	#if defined(MATH_INTEL_SIMD)
		mm = _mm_mul_ps(mm, b.mm);
	#else
		x() *= b.x();
		y() *= b.y();
		z() *= b.z();
		w() *= b.w();
	#endif
	return SELF;
}

// /
inline Vec4 Vec4::operator/(const Vec4& b) const
{
	#if defined(MATH_INTEL_SIMD)
		return Vec4(_mm_div_ps(mm, b.mm));
	#else
		return Vec4(x() / b.x(), y() / b.y(), z() / b.z(), w() / b.w());
	#endif
}

// /=
inline Vec4& Vec4::operator/=(const Vec4& b)
{
	#if defined(MATH_INTEL_SIMD)
		mm = _mm_div_ps(mm, b.mm);
	#else
		x() /= b.x();
		y() /= b.y();
		z() /= b.z();
		w() /= b.w();
	#endif
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
	Vec4 sub = SELF - b;
	return isZero(sub.x()) && isZero(sub.y()) && isZero(sub.z()) && isZero(sub.w());
}

// !=
inline bool Vec4::operator!=(const Vec4& b) const
{
	Vec4 sub = SELF - b;
	return !(isZero(sub.x()) && isZero(sub.y()) && isZero(sub.z()) && isZero(sub.w()));
}


//======================================================================================================================
// Operators with float                                                                                                =
//======================================================================================================================

// Vec4 + float
inline Vec4 Vec4::operator+(float f) const
{
	return SELF + Vec4(f);
}

// float + Vec4
inline Vec4 operator+(float f, const Vec4& v4)
{
	return v4 + f;
}

// Vec4 += float
inline Vec4& Vec4::operator+=(float f)
{
	SELF += Vec4(f);
	return SELF;
}

// Vec4 - float
inline Vec4 Vec4::operator-(float f) const
{
	return SELF - Vec4(f);
}

// float - Vec4
inline Vec4 operator-(float f, const Vec4& v4)
{
	return Vec4(f) - v4;
}

// Vec4 -= float
inline Vec4& Vec4::operator-=(float f)
{
	SELF -= Vec4(f);
	return SELF;
}

// Vec4 * float
inline Vec4 Vec4::operator*(float f) const
{
	return SELF * Vec4(f);
}

// float * Vec4
inline Vec4 operator*(float f, const Vec4& v4)
{
	return v4 * f;
}

// Vec4 *= float
inline Vec4& Vec4::operator*=(float f)
{
	SELF *= Vec4(f);
	return SELF;
}

// Vec4 / float
inline Vec4 Vec4::operator/(float f) const
{
	return SELF / Vec4(f);
}

// float / Vec4
inline Vec4 operator/(float f, const Vec4& v4)
{
	return Vec4(f) / v4;
}

// Vec4 /= float
inline Vec4& Vec4::operator/=(float f)
{
	SELF /= Vec4(f);
	return SELF;
}


//======================================================================================================================
// Operators with other                                                                                                =
//======================================================================================================================

// Vec4 * mat4
inline Vec4 Vec4::operator*(const Mat4& m4) const
{
	return Vec4(
		x() * m4(0, 0) + y() * m4(1, 0) + z() * m4(2, 0) + w() * m4(3, 0),
		x() * m4(0, 1) + y() * m4(1, 1) + z() * m4(2, 1) + w() * m4(3, 1),
		x() * m4(0, 2) + y() * m4(1, 2) + z() * m4(2, 2) + w() * m4(3, 2),
		x() * m4(0, 3) + y() * m4(1, 3) + z() * m4(2, 3) + w() * m4(3, 3)
	);
}


//======================================================================================================================
// Misc methods                                                                                                        =
//======================================================================================================================

// dot
inline float Vec4::dot(const Vec4& b) const
{
	#if defined(MATH_INTEL_SIMD)
		float o;
		_mm_store_ss(&o, _mm_dp_ps(mm, b.mm, 0xF1));
		return o;
	#else
		return x() * b.x() + y() * b.y() + z() * b.z() + w() * b.w();
	#endif
}

// getLength
inline float Vec4::getLength() const
{
	return M::sqrt(dot(SELF));
}

// getNormalized
inline Vec4 Vec4::getNormalized() const
{
	return SELF / getLength();
}

// normalize
inline void Vec4::normalize()
{
	SELF /= getLength();
}


//======================================================================================================================
// Print                                                                                                               =
//======================================================================================================================
inline std::ostream& operator<<(std::ostream& s, const Vec4& v)
{
	s << v.x() << ' ' << v.y() << ' ' << v.z() << ' ' << v.w();
	return s;
}


} // end namespace
