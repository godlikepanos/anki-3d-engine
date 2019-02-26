// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
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

/// 4D vector. SIMD optimized
template<typename T>
class alignas(16) TVec4 : public TVec<T, 4, TVec4<T>>
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
	using Base = TVec<T, 4, TVec4<T>>;

	using Base::w;
	using Base::x;
	using Base::y;
	using Base::z;
	using Base::operator*;

	/// @name Constructors
	/// @{
	TVec4()
		: Base()
	{
	}

	TVec4(const TVec4& b)
		: Base(b)
	{
	}

	TVec4(const T x_, const T y_, const T z_, const T w_)
		: Base(x_, y_, z_, w_)
	{
	}

	explicit TVec4(const T f)
		: Base(f)
	{
	}

	explicit TVec4(const T arr[])
		: Base(arr)
	{
	}

	explicit TVec4(const typename Base::Simd& simd)
		: Base(simd)
	{
	}

	TVec4(const TVec2<T>& v, const T z_, const T w_)
		: Base(v.x(), v.y(), z_, w_)
	{
	}

	TVec4(const TVec2<T>& v, const TVec2<T>& v2)
		: Base(v.x(), v.y(), v2.x(), v2.y())
	{
	}

	TVec4(const TVec3<T>& v, const T w_)
		: Base(v.x(), v.y(), v.z(), w_)
	{
	}
	/// @}

	/// @name Operators with other
	/// @{

	/// @note 16 muls 12 adds
	ANKI_USE_RESULT TVec4 operator*(const TMat4<T>& m4) const
	{
		return TVec4(x() * m4(0, 0) + y() * m4(1, 0) + z() * m4(2, 0) + w() * m4(3, 0),
			x() * m4(0, 1) + y() * m4(1, 1) + z() * m4(2, 1) + w() * m4(3, 1),
			x() * m4(0, 2) + y() * m4(1, 2) + z() * m4(2, 2) + w() * m4(3, 2),
			x() * m4(0, 3) + y() * m4(1, 3) + z() * m4(2, 3) + w() * m4(3, 3));
	}
	/// @}
};

/// F32 4D vector
using Vec4 = TVec4<F32>;
static_assert(sizeof(Vec4) == sizeof(F32) * 4, "Incorrect size");

/// Half float 4D vector
using HVec4 = TVec4<F16>;

/// 32bit signed integer 4D vector
using IVec4 = TVec4<I32>;

/// 32bit unsigned integer 4D vector
using UVec4 = TVec4<U32>;
/// @}

} // end namespace anki

#include <anki/math/Vec4.inl.h>
