// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/math/CommonIncludes.h>
#include <anki/math/Vec.h>

namespace anki
{

/// @addtogroup math
/// @{

/// 2D vector
template<typename T>
class TVec2 : public TVec<T, 2, Array<T, 2>, TVec2<T>>
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
	TVec2()
		: Base()
	{
	}

	TVec2(const TVec2& b)
		: Base(b)
	{
	}

	TVec2(const T x_, const T y_)
		: Base(x_, y_)
	{
	}

	explicit TVec2(const T f)
		: Base(f)
	{
	}

	explicit TVec2(const T arr[])
		: Base(arr)
	{
	}
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
using Vec2 = TVec2<F32>;
static_assert(sizeof(Vec2) == sizeof(F32) * 2, "Incorrect size");

/// Half float 2D vector
using HVec2 = TVec2<F16>;

/// 32bit signed integer 2D vector
using IVec2 = TVec2<I32>;

/// 32bit unsigned integer 2D vector
using UVec2 = TVec2<U32>;
/// @}

} // end namespace anki
