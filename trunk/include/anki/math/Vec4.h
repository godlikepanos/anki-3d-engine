#ifndef ANKI_MATH_VEC4_H
#define ANKI_MATH_VEC4_H

#include "anki/math/CommonIncludes.h"

namespace anki {

/// @addtogroup Math
/// @{

/// Template struct that gives the type of the TVec4 SIMD
template<typename T>
struct TVec4Simd
{
	typedef Array<T, 4> Type;
};

#if ANKI_SIMD == ANKI_SIMD_SSE
// Specialize for F32
template<>
struct TVec4Simd<F32>
{
	typedef __m128 Type;
};
#elif ANKI_SIMD == ANKI_SIMD_NEON
// Specialize for F32
template<>
struct TVec4Simd<F32>
{
	typedef float32x4_t Type;
};
#endif

/// 4D vector. SIMD optimized
template<typename T>
ANKI_ATTRIBUTE_ALIGNED(class, 16) TVec4
{
	/// @name Friends
	/// @{
	template<typename Y>
	friend TVec4<Y> operator+(const Y f, const TVec4<Y>& v4);
	template<typename Y>
	friend TVec4<Y> operator-(const Y f, const TVec4<Y>& v4);
	template<typename Y>
	friend TVec4<Y> operator*(const Y f, const TVec4<Y>& v4);
	template<typename Y>
	friend TVec4<Y> operator/(const Y f, const TVec4<Y>& v4);
	/// @}

public:
	typedef typename TVec4Simd<T>::Type Simd;

	/// @name Constructors
	/// @{
	explicit TVec4()
	{}

	explicit TVec4(const T x_, const T y_, const T z_, const T w_)
	{
		x() = x_;
		y() = y_;
		z() = z_;
		w() = w_;
	}

	explicit TVec4(const T f)
	{
		arr[0] = arr[1] = arr[2] = arr[3] = f;
	}

	explicit TVec4(const T arr_[])
	{
		arr[0] = arr_[0];
		arr[1] = arr_[1];
		arr[2] = arr_[2];
		arr[3] = arr_[3];
	}

	explicit TVec4(const TVec2<T>& v2, const T z_, const T w_)
	{
		x() = v2.x();
		y() = v2.y();
		z() = z_;
		w() = w_;
	}

	explicit TVec4(const TVec2<T>& av2, const TVec2<T>& bv2)
	{
		x() = av2.x();
		y() = av2.y();
		z() = bv2.x();
		w() = bv2.y();
	}

	explicit TVec4(const TVec3<T>& v3, const T w_)
	{
		x() = v3.x();
		y() = v3.y();
		z() = v3.z();
		w() = w_;
	}

	TVec4(const TVec4& b)
	{
		x() = b.x();
		y() = b.y();
		z() = b.z();
		w() = b.w();
	}

	explicit TVec4(const TQuat<T>& q)
	{
		x() = q.x();
		y() = q.y();
		z() = q.z();
		w() = q.w();
	}

	explicit TVec4(const Simd& simd_)
	{
		simd = simd_;
	}
	/// @}

	/// @name Accessors
	/// @{
	T& x()
	{
		return vec.x;
	}

	T x() const
	{
		return vec.x;
	}

	T& y()
	{
		return vec.y;
	}

	T y() const
	{
		return vec.y;
	}

	T& z()
	{
		return vec.z;
	}

	T z() const
	{
		return vec.z;
	}

	T& w()
	{
		return vec.w;
	}

	T w() const
	{
		return vec.w;
	}

	T& operator[](const U i)
	{
		return arr[i];
	}

	T operator[](const U i) const
	{
		return arr[i];
	}

	TVec2<T> xy() const
	{
		return TVec2<T>(x(), y());
	}

	TVec3<T> xyz() const
	{
		return TVec3<T>(x(), y(), z());
	}

	Simd& getSimd()
	{
		return simd;
	}

	const Simd& getSimd() const
	{
		return simd;
	}
	/// @}

	/// @name Operators with same type
	/// @{
	TVec4& operator=(const TVec4& b)
	{
		x() = b.x();
		y() = b.y();
		z() = b.z();
		w() = b.w();
		return (*this);
	}

	TVec4 operator+(const TVec4& b) const
	{
		return TVec4(x() + b.x(), y() + b.y(), z() + b.z(), w() + b.w());
	}

	TVec4& operator+=(const TVec4& b)
	{
		x() += b.x();
		y() += b.y();
		z() += b.z();
		w() += b.w();
		return (*this);
	}

	TVec4 operator-(const TVec4& b) const
	{
		return TVec4(x() - b.x(), y() - b.y(), z() - b.z(), w() - b.w());
	}

	TVec4& operator-=(const TVec4& b)
	{
		x() -= b.x();
		y() -= b.y();
		z() -= b.z();
		w() -= b.w();
		return (*this);
	}

	TVec4 operator*(const TVec4& b) const
	{
		return TVec4(x() * b.x(), y() * b.y(), z() * b.z(), w() * b.w());
	}

	TVec4& operator*=(const TVec4& b)
	{
		x() *= b.x();
		y() *= b.y();
		z() *= b.z();
		w() *= b.w();
		return (*this);
	}

	TVec4 operator/(const TVec4& b) const
	{
		return TVec4(x() / b.x(), y() / b.y(), z() / b.z(), w() / b.w());
	}

	TVec4& operator/=(const TVec4& b)
	{
		x() /= b.x();
		y() /= b.y();
		z() /= b.z();
		w() /= b.w();
		return (*this);
	}

	TVec4 operator-() const
	{
		return TVec4(-x(), -y(), -z(), -w());
	}

	Bool operator==(const TVec4& b) const
	{
		TVec4 sub = (*this) - b;
		return isZero<T>(sub.x()) 
			&& isZero<T>(sub.y()) 
			&& isZero<T>(sub.z()) 
			&& isZero<T>(sub.w());
	}

	Bool operator!=(const TVec4& b) const
	{
		return !operator==(b);
	}

	Bool operator<(const TVec4& b) const
	{
		return x() < b.x() && y() < b.y() && z() < b.z() && w() < b.w();
	}

	Bool operator<=(const TVec4& b) const
	{
		return x() <= b.x() && y() <= b.y() && z() <= b.z() && w() <= b.w();
	}

	Bool operator>(const TVec4& b) const
	{
		return x() > b.x() && y() > b.y() && z() > b.z() && w() > b.w();
	}

	Bool operator>=(const TVec4& b) const
	{
		return x() >= b.x() && y() >= b.y() && z() >= b.z() && w() >= b.w();
	}
	/// @}

	/// @name Operators with T
	/// @{
	TVec4 operator+(const T f) const
	{
		return (*this) + TVec4(f);
	}

	TVec4& operator+=(const T f)
	{
		(*this) += TVec4(f);
		return (*this);
	}

	TVec4 operator-(const T f) const
	{
		return (*this) - TVec4(f);
	}

	TVec4& operator-=(const T f)
	{
		(*this) -= TVec4(f);
		return (*this);
	}

	TVec4 operator*(const T f) const
	{
		return (*this) * TVec4(f);
	}

	TVec4& operator*=(const T f)
	{
		(*this) *= TVec4(f);
		return (*this);
	}

	TVec4 operator/(const T f) const
	{
		return (*this) / TVec4(f);
	}

	TVec4& operator/=(const T f)
	{
		(*this) /= TVec4(f);
		return (*this);
	}
	/// @}

	/// @name Operators with other
	/// @{

	/// @note 16 muls 12 adds
	TVec4 operator*(const TMat4<T>& m4) const
	{
		return TVec4(
			x() * m4(0, 0) + y() * m4(1, 0) + z() * m4(2, 0) + w() * m4(3, 0),
			x() * m4(0, 1) + y() * m4(1, 1) + z() * m4(2, 1) + w() * m4(3, 1),
			x() * m4(0, 2) + y() * m4(1, 2) + z() * m4(2, 2) + w() * m4(3, 2),
			x() * m4(0, 3) + y() * m4(1, 3) + z() * m4(2, 3) + w() * m4(3, 3));
	}
	/// @}

	/// @name Other
	/// @{
	T getLength() const
	{
		return sqrt<T>(dot(*this));
	}

	TVec4 getNormalized() const
	{
		return (*this) / getLength();
	}

	void normalize()
	{
		(*this) /= getLength();
	}

	T dot(const TVec4& b) const
	{
		return x() * b.x() + y() * b.y() + z() * b.z() + w() * b.w();
	}

	std::string toString() const
	{
		return std::to_string(x()) 
			+ " " + std::to_string(y())
			+ " " + std::to_string(z())
			+ " " + std::to_string(w());
	}
	/// @}

private:
	/// @name Data
	/// @{
	union
	{
		struct
		{
			T x, y, z, w;
		} vec;

		Array<T, 4> arr;

		Simd simd;
	};
	/// @}
};

#if ANKI_SIMD == ANKI_SIMD_SSE

// Forward declare specializations

template<>
TVec4<F32>::TVec4(F32 f);

template<>
TVec4<F32>::TVec4(const F32 arr_[]);

template<>
TVec4<F32>::TVec4(const F32 x_, const F32 y_, const F32 z_, const F32 w_);

template<>
TVec4<F32>::TVec4(const TVec4<F32>& b);

template<>
TVec4<F32>& TVec4<F32>::operator=(const TVec4<F32>& b);

template<>
TVec4<F32> TVec4<F32>::operator+(const TVec4<F32>& b) const;

template<>
TVec4<F32>& TVec4<F32>::operator+=(const TVec4<F32>& b);

template<>
TVec4<F32> TVec4<F32>::operator-(const TVec4<F32>& b) const;

template<>
TVec4<F32>& TVec4<F32>::operator-=(const TVec4<F32>& b);

template<>
TVec4<F32> TVec4<F32>::operator*(const TVec4<F32>& b) const;

template<>
TVec4<F32>& TVec4<F32>::operator*=(const TVec4<F32>& b);

template<>
TVec4<F32> TVec4<F32>::operator/(const TVec4<F32>& b) const;

template<>
TVec4<F32>& TVec4<F32>::operator/=(const TVec4<F32>& b);

template<>
F32 TVec4<F32>::dot(const TVec4<F32>& b) const;

template<>
TVec4<F32> TVec4<F32>::getNormalized() const;

template<>
void TVec4<F32>::normalize();

#elif ANKI_SIMD == ANKI_SIMD_NEON

template<>
TVec4<F32>::TVec4(F32 f);

template<>
TVec4<F32>::TVec4(const F32 arr_[]);

template<>
TVec4<F32>::TVec4(const TVec4<F32>& b);

template<>
TVec4<F32>& TVec4<F32>::operator=(const TVec4<F32>& b);

template<>
TVec4<F32> TVec4<F32>::operator+(const TVec4<F32>& b) const;

template<>
TVec4<F32>& TVec4<F32>::operator+=(const TVec4<F32>& b);

template<>
TVec4<F32> TVec4<F32>::operator-(const TVec4<F32>& b) const;

template<>
TVec4<F32>& TVec4<F32>::operator-=(const TVec4<F32>& b);

template<>
TVec4<F32> TVec4<F32>::operator*(const TVec4<F32>& b) const;

template<>
TVec4<F32>& TVec4<F32>::operator*=(const TVec4<F32>& b);

template<>
TVec4<F32> operator*(const F32 f) const;

template<>
TVec4<F32>& operator*=(const F32 f);

template<>
F32 TVec4<F32>::dot(const TVec4<F32>& b) const;

template<>
TVec4<F32> TVec4<F32>::getNormalized() const;

template<>
void TVec4<F32>::normalize();

#endif

/// F32 4D vector
typedef TVec4<F32> Vec4;
static_assert(sizeof(Vec4) == sizeof(F32) * 4, "Incorrect size");

/// Half float 4D vector
typedef TVec4<F16> HVec4;

/// 32bit signed integer 4D vector
typedef TVec4<I32> IVec4;

/// 32bit unsigned integer 4D vector
typedef TVec4<U32> UVec4;
/// @}

} // end namespace anki

#include "anki/math/Vec4.inl.h"

#endif
