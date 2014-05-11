#ifndef ANKI_MATH_VEC2_H
#define ANKI_MATH_VEC2_H

#include "anki/math/CommonIncludes.h"
#include "anki/math/Vec.h"

namespace anki {

/// @addtogroup math
/// @{

/// 2D vector
template<typename T>
class TVec2: public TVec<T, 2, Array<T, 2>, TVec2<T>>
{
	/// @name Friends
	/// @{
	template<typename Y>
	friend TVec2<Y> operator+(const Y f, const TVec2<Y>& b);
	template<typename Y>
	friend TVec2<Y> operator-(const Y f, const TVec2<Y>& b);
	template<typename Y>
	friend TVec2<Y> operator*(const Y f, const TVec2<Y>& b);
	template<typename Y>
	friend TVec2<Y> operator/(const Y f, const TVec2<Y>& b);
	///@}

public:
	using Base = TVec<T, 2, Array<T, 2>, TVec2<T>>;

	/// @name Constructors
	/// @{
	explicit TVec2()
		: Base()
	{}

	TVec2(const TVec2& b)
		: Base(b)
	{}

	explicit TVec2(const T x_, const T y_)
		: Base(x_, y_)
	{}

	explicit TVec2(const T f)
		: Base(f)
	{}

	explicit TVec2(const T arr[])
		: Base(arr)
	{}
	/// @}
};

/// @memberof TVec2
template<typename T>
TVec2<T> operator+(const T f, const TVec2<T>& b)
{
	return b + f;
}

/// @memberof TVec2
template<typename T>
TVec2<T> operator-(const T f, const TVec2<T>& b)
{
	return TVec2<T>(f - b.x(), f - b.y());
}

/// @memberof TVec2
template<typename T>
TVec2<T> operator*(const T f, const TVec2<T>& b)
{
	return b * f;
}

/// @memberof TVec2
template<typename T>
TVec2<T> operator/(const T f, const TVec2<T>& b)
{
	return TVec2<T>(f / b.x(), f / b.y());
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
