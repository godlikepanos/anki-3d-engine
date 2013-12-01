#ifndef ANKI_MATH_VEC2_H
#define ANKI_MATH_VEC2_H

#include "anki/math/CommonIncludes.h"

namespace anki {

/// @addtogroup Math
/// @{

/// 2D vector
template<typename T>
class TVec2
{
	/// @name Friends
	template<typename Y>
	friend TVec2<Y> operator+(const Y f, const TVec2<Y>& v2);
	template<typename Y>
	friend TVec2<Y> operator-(const Y f, const TVec2<Y>& v2);
	template<typename Y>
	friend TVec2<Y> operator*(const Y f, const TVec2<Y>& v2);
	template<typename Y>
	friend TVec2<Y> operator/(const Y f, const TVec2<Y>& v2);
	///@]

public:
	/// @name Constructors
	/// @{
	explicit TVec2()
	{}

	explicit TVec2(const T x_, const T y_)
	{
		x() = x_;
		y() = y_;
	}
	
	explicit TVec2(const T f)
	{
		x() = y() = f;
	}

	explicit TVec2(const T arr_[])
	{
		x() = arr_[0];
		y() = arr_[1];
	}

	TVec2(const TVec2& b)
	{
		x() = b.x();
		y() = b.y();
	}

	explicit TVec2(const TVec3<T>& v3)
	{
		x() = v3.x();
		y() = v3.y();
	}

	explicit TVec2(const TVec4<T>& v4)
	{
		x() = v4.x();
		y() = v4.y();
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

	TVec2 xx() const
	{
		return TVec2(vec.x, vec.x);
	}

	TVec2 yy() const
	{
		return TVec2(vec.y, vec.y);
	}

	TVec2 xy() const
	{
		return *this;
	}

	TVec2 yx() const
	{
		return TVec2(vec.y, vec.x);
	}

	T& operator[](const U i)
	{
		return arr[i];
	}

	T operator[](const U i) const
	{
		return arr[i];
	}
	/// @}

	/// @name Operators with same type
	/// @{
	TVec2& operator=(const TVec2& b)
	{
		x() = b.x();
		y() = b.y();
		return *this;
	}

	TVec2 operator+(const TVec2& b) const
	{
		return TVec2(x() + b.x(), y() + b.y());
	}

	TVec2& operator+=(const TVec2& b)
	{
		x() += b.x();
		y() += b.y();
		return (*this);
	}

	TVec2 operator-(const TVec2& b) const
	{
		return TVec2(x() - b.x(), y() - b.y());
	}

	TVec2& operator-=(const TVec2& b)
	{
		x() -= b.x();
		y() -= b.y();
		return (*this);
	}

	TVec2 operator*(const TVec2& b) const
	{
		return TVec2(x() * b.x(), y() * b.y());
	}

	TVec2& operator*=(const TVec2& b)
	{
		x() *= b.x();
		y() *= b.y();
		return (*this);
	}

	TVec2 operator/(const TVec2& b) const
	{
		return TVec2(x() / b.x(), y() / b.y());
	}

	TVec2& operator/=(const TVec2& b)
	{
		x() /= b.x();
		y() /= b.y();
		return (*this);
	}

	TVec2 operator-() const
	{
		return TVec2(-x(), -y());
	}

	Bool operator==(const TVec2& b) const
	{
		return isZero<T>(x() - b.x()) && isZero<T>(y() - b.y());
	}

	Bool operator!=(const TVec2& b) const
	{
		return !(*this == b);
	}

	Bool operator<(const TVec2& b) const
	{
		return vec.x < b.vec.x && vec.y < b.vec.y;
	}

	Bool operator<=(const TVec2& b) const
	{
		return vec.x <= b.vec.x && vec.y <= b.vec.y;
	}

	Bool operator>(const TVec2& b) const
	{
		return vec.x > b.vec.x && vec.y > b.vec.y;
	}

	Bool operator>=(const TVec2& b) const
	{
		return vec.x >= b.vec.x && vec.y >= b.vec.y;
	}
	/// @}

	/// @name Operators with T
	/// @{
	TVec2 operator+(const T f) const
	{
		return (*this) + TVec2(f);
	}

	TVec2& operator+=(const T f)
	{
		(*this) += TVec2(f);
		return (*this);
	}

	TVec2 operator-(const T f) const
	{
		return (*this) - TVec2(f);
	}

	TVec2& operator-=(const T f)
	{
		(*this) -= TVec2(f);
		return (*this);
	}

	TVec2 operator*(const T f) const
	{
		return (*this) * TVec2(f);
	}

	TVec2& operator*=(const T f)
	{
		(*this) *= TVec2(f);
		return (*this);
	}

	TVec2 operator/(const T f) const
	{
		return (*this) / TVec2(f);
	}

	TVec2& operator/=(const T f)
	{
		(*this) /= TVec2(f);
		return (*this);
	}
	/// @}

	/// @name Other
	/// @{
	T getLengthSquared() const
	{
		return x() * x() + y() * y();
	}

	T getLength() const
	{
		return sqrt<T>(getLengthSquared());
	}

	TVec2 getNormalized() const
	{
		return (*this) / getLength();
	}

	void normalize()
	{
		(*this) /= getLength();
	}

	T dot(const TVec2& b) const
	{
		return x() * b.x() + y() * b.y();
	}

	std::string toString() const
	{
		return std::to_string(x()) + " " + std::to_string(y());
	}
	/// @}

private:
	/// @name Data members
	/// @{
	struct Vec
	{
		T x, y;
	};

	union
	{
		Vec vec;
		Array<T, 2> arr;
	};
	/// @}
};

template<typename T>
TVec2<T> operator+(const T f, const TVec2<T>& v2)
{
	return v2 + f;
}

template<typename T>
TVec2<T> operator-(const T f, const TVec2<T>& v2)
{
	return TVec2<T>(f - v2.x(), f - v2.y());
}

template<typename T>
TVec2<T> operator*(const T f, const TVec2<T>& v2)
{
	return v2 * f;
}

template<typename T>
TVec2<T> operator/(const T f, const TVec2<T>& v2)
{
	return TVec2<T>(f / v2.x(), f / v2.y());
}

/// F32 2D vector
typedef TVec2<F32> Vec2;
static_assert(sizeof(Vec2) == sizeof(F32) * 2, "Incorrect size");

/// Half float 2D vector
typedef TVec2<F16> HVec2;

/// 32bit signed integer 2D vector 
typedef TVec2<I32> IVec2;

/// 32bit unsigned integer 2D vector 
typedef TVec2<U32> UVec2;

/// @}

} // end namespace anki

#endif
