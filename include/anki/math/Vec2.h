#ifndef ANKI_MATH_VEC2_H
#define ANKI_MATH_VEC2_H

#include "anki/math/CommonIncludes.h"

namespace anki {

/// @addtogroup Math
/// @{

/// XXX
template<typename T>
class TVec2
{
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

	explicit TVec2(const T arr[])
	{
		x() = arr[0];
		y() = arr[1];
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
		return sqrt(getLengthSquared());
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
	/// @}

	/// @name Friends
	friend TVec2 operator+(const T f, const TVec2& v2)
	{
		return v2 + f;
	}

	friend TVec2 operator-(const T f, const TVec2& v2)
	{
		return TVec2(f - v2.x(), f - v2.y());
	}

	friend TVec2 operator*(const T f, const TVec2& v2)
	{
		return v2 * f;
	}

	friend TVec2 operator/(const T f, const TVec2& v2)
	{
		return TVec2(f / v2.x(), f / v2.y());
	}

	friend std::ostream& operator<<(std::ostream& s, const TVec2& v)
	{
		s << v.x() << ' ' << v.y();
		return s;
	}
	///@]

private:
	/// @name Data members
	/// @{
	union
	{
		struct
		{
			T x, y;
		} vec;

		Array<T, 2> arr;
	};
	/// @}
};

/// 2D vector
class Vec2
{
public:
	/// @name Constructors
	/// @{
	explicit Vec2();
	explicit Vec2(const F32 x, const F32 y);
	explicit Vec2(const F32 f);
	explicit Vec2(const F32 arr[]);
	Vec2(const Vec2& b);
	explicit Vec2(const Vec3& v3);
	explicit Vec2(const Vec4& v4);
	/// @}

	/// @name Accessors
	/// @{
	F32& x();
	F32 x() const;
	F32& y();
	F32 y() const;
	F32& operator[](const U i);
	F32 operator[](const U i) const;
	/// @}

	/// @name Operators with same type
	/// @{
	Vec2& operator=(const Vec2& b);
	Vec2 operator+(const Vec2& b) const;
	Vec2& operator+=(const Vec2& b);
	Vec2 operator-(const Vec2& b) const;
	Vec2& operator-=(const Vec2& b);
	Vec2 operator*(const Vec2& b) const;
	Vec2& operator*=(const Vec2& b);
	Vec2 operator/(const Vec2& b) const;
	Vec2& operator/=(const Vec2& b);
	Vec2 operator-() const;
	Bool operator==(const Vec2& b) const;
	Bool operator!=(const Vec2& b) const;
	Bool operator<(const Vec2& b) const;
	Bool operator<=(const Vec2& b) const;
	Bool operator>(const Vec2& b) const;
	Bool operator>=(const Vec2& b) const;
	/// @}

	/// @name Operators with F32
	/// @{
	Vec2 operator+(const F32 f) const;
	Vec2& operator+=(const F32 f);
	Vec2 operator-(const F32 f) const;
	Vec2& operator-=(const F32 f);
	Vec2 operator*(const F32 f) const;
	Vec2& operator*=(const F32 f);
	Vec2 operator/(const F32 f) const;
	Vec2& operator/=(const F32 f);
	/// @}

	/// @name Other
	/// @{
	F32 getLengthSquared() const;
	F32 getLength() const;
	Vec2 getNormalized() const;
	void normalize();
	F32 dot(const Vec2& b) const;
	/// @}

	/// @name Friends
	friend Vec2 operator+(const F32 f, const Vec2& v2);
	friend Vec2 operator-(const F32 f, const Vec2& v2);
	friend Vec2 operator*(const F32 f, const Vec2& v2);
	friend Vec2 operator/(const F32 f, const Vec2& v2);
	friend std::ostream& operator<<(std::ostream& s, const Vec2& v);
	///@]

private:
	/// @name Data members
	/// @{
	union
	{
		struct
		{
			F32 x, y;
		} vec;

		Array<F32, 2> arr;
	};
	/// @}
};
/// @}

static_assert(sizeof(Vec2) == sizeof(F32) * 2, "Incorrect size");

} // end namespace anki

#include "anki/math/Vec2.inl.h"

#endif
