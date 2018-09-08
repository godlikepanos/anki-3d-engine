// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/math/CommonIncludes.h>
#include <type_traits>

namespace anki
{

/// @addtogroup math
/// @{

/// Common code for all vectors
template<typename T, U N, typename TSimd, typename TV>
class TVec
{
public:
	using Scalar = T;
	using Simd = TSimd;
	static constexpr U SIZE = N;
	static constexpr Bool IS_INTEGER = std::is_integral<T>::value;

	/// @name Constructors
	/// @{
	TVec()
	{
	}

	TVec(const TVec& b)
	{
		for(U i = 0; i < N; i++)
		{
			m_arr[i] = b.m_arr[i];
		}
	}

	TVec(const T x_, const T y_)
	{
		static_assert(N == 2, "Wrong vector");
		x() = x_;
		y() = y_;
	}

	TVec(const T x_, const T y_, const T z_)
	{
		static_assert(N == 3, "Wrong vector");
		x() = x_;
		y() = y_;
		z() = z_;
	}

	TVec(const T x_, const T y_, const T z_, const T w_)
	{
		static_assert(N == 4, "Wrong vector");
		x() = x_;
		y() = y_;
		z() = z_;
		w() = w_;
	}

	explicit TVec(const T f)
	{
		for(U i = 0; i < N; ++i)
		{
			m_arr[i] = f;
		}
	}

	explicit TVec(const T arr[])
	{
		for(U i = 0; i < N; ++i)
		{
			m_arr[i] = arr[i];
		}
	}

	explicit TVec(const Simd& simd)
	{
		m_simd = simd;
	}
	/// @}

	/// @name Accessors
	/// @{
	T& x()
	{
		return m_arr[0];
	}

	T x() const
	{
		return m_arr[0];
	}

	T& y()
	{
		return m_arr[1];
	}

	T y() const
	{
		return m_arr[1];
	}

	T& z()
	{
		static_assert(N > 2, "Wrong vector");
		return m_arr[2];
	}

	T z() const
	{
		static_assert(N > 2, "Wrong vector");
		return m_arr[2];
	}

	T& w()
	{
		static_assert(N > 3, "Wrong vector");
		return m_arr[3];
	}

	T w() const
	{
		static_assert(N > 3, "Wrong vector");
		return m_arr[3];
	}

	TVec2<T> xx() const
	{
		return TVec2<T>(x(), x());
	}

	TVec2<T> yy() const
	{
		return TVec2<T>(y(), y());
	}

	TVec2<T> xy() const
	{
		return TVec2<T>(x(), y());
	}

	TVec2<T> yx() const
	{
		return TVec2<T>(y(), x());
	}

	TVec2<T> zw() const
	{
		static_assert(N == 4, "Wrong vector");
		return TVec2<T>(z(), w());
	}

	TVec3<T> xxx() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(x(), x(), x());
	}

	TVec3<T> xxy() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(x(), x(), y());
	}

	TVec3<T> xxz() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(x(), x(), z());
	}

	TVec3<T> xyx() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(x(), y(), x());
	}

	TVec3<T> xyy() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(x(), y(), y());
	}

	TVec3<T> xyz() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(x(), y(), z());
	}

	TVec3<T> xzx() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(x(), z(), x());
	}

	TVec3<T> xzy() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(x(), z(), y());
	}

	TVec3<T> xzz() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(x(), z(), z());
	}

	TVec3<T> yxx() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(y(), x(), x());
	}

	TVec3<T> yxy() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(y(), x(), y());
	}

	TVec3<T> yxz() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(y(), x(), z());
	}

	TVec3<T> yyx() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(y(), y(), x());
	}

	TVec3<T> yyy() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(y(), y(), y());
	}

	TVec3<T> yyz() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(y(), y(), z());
	}

	TVec3<T> yzx() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(y(), z(), x());
	}

	TVec3<T> yzy() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(y(), z(), y());
	}

	TVec3<T> yzz() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(y(), z(), z());
	}

	TVec3<T> zxx() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(z(), x(), x());
	}

	TVec3<T> zxy() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(z(), x(), y());
	}

	TVec3<T> zxz() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(z(), x(), z());
	}

	TVec3<T> zyx() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(z(), y(), x());
	}

	TVec3<T> zyy() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(z(), y(), y());
	}

	TVec3<T> zyz() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(z(), y(), z());
	}

	TVec3<T> zzx() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(z(), z(), x());
	}

	TVec3<T> zzy() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(z(), z(), y());
	}

	TVec3<T> zzz() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec3<T>(z(), z(), z());
	}

	TVec4<T> xxxx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), x(), x(), x());
	}

	TVec4<T> xxxy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), x(), x(), y());
	}

	TVec4<T> xxxz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), x(), x(), z());
	}

	TVec4<T> xxxw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), x(), x(), w());
	}

	TVec4<T> xxyx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), x(), y(), x());
	}

	TVec4<T> xxyy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), x(), y(), y());
	}

	TVec4<T> xxyz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), x(), y(), z());
	}

	TVec4<T> xxyw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), x(), y(), w());
	}

	TVec4<T> xxzx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), x(), z(), x());
	}

	TVec4<T> xxzy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), x(), z(), y());
	}

	TVec4<T> xxzz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), x(), z(), z());
	}

	TVec4<T> xxzw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), x(), z(), w());
	}

	TVec4<T> xxwx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), x(), w(), x());
	}

	TVec4<T> xxwy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), x(), w(), y());
	}

	TVec4<T> xxwz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), x(), w(), z());
	}

	TVec4<T> xxww() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), x(), w(), w());
	}

	TVec4<T> xyxx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), y(), x(), x());
	}

	TVec4<T> xyxy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), y(), x(), y());
	}

	TVec4<T> xyxz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), y(), x(), z());
	}

	TVec4<T> xyxw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), y(), x(), w());
	}

	TVec4<T> xyyx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), y(), y(), x());
	}

	TVec4<T> xyyy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), y(), y(), y());
	}

	TVec4<T> xyyz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), y(), y(), z());
	}

	TVec4<T> xyyw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), y(), y(), w());
	}

	TVec4<T> xyzx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), y(), z(), x());
	}

	TVec4<T> xyzy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), y(), z(), y());
	}

	TVec4<T> xyzz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), y(), z(), z());
	}

	TVec4<T> xyzw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), y(), z(), w());
	}

	TVec4<T> xywx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), y(), w(), x());
	}

	TVec4<T> xywy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), y(), w(), y());
	}

	TVec4<T> xywz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), y(), w(), z());
	}

	TVec4<T> xyww() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), y(), w(), w());
	}

	TVec4<T> xzxx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), z(), x(), x());
	}

	TVec4<T> xzxy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), z(), x(), y());
	}

	TVec4<T> xzxz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), z(), x(), z());
	}

	TVec4<T> xzxw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), z(), x(), w());
	}

	TVec4<T> xzyx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), z(), y(), x());
	}

	TVec4<T> xzyy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), z(), y(), y());
	}

	TVec4<T> xzyz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), z(), y(), z());
	}

	TVec4<T> xzyw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), z(), y(), w());
	}

	TVec4<T> xzzx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), z(), z(), x());
	}

	TVec4<T> xzzy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), z(), z(), y());
	}

	TVec4<T> xzzz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), z(), z(), z());
	}

	TVec4<T> xzzw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), z(), z(), w());
	}

	TVec4<T> xzwx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), z(), w(), x());
	}

	TVec4<T> xzwy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), z(), w(), y());
	}

	TVec4<T> xzwz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), z(), w(), z());
	}

	TVec4<T> xzww() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), z(), w(), w());
	}

	TVec4<T> xwxx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), w(), x(), x());
	}

	TVec4<T> xwxy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), w(), x(), y());
	}

	TVec4<T> xwxz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), w(), x(), z());
	}

	TVec4<T> xwxw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), w(), x(), w());
	}

	TVec4<T> xwyx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), w(), y(), x());
	}

	TVec4<T> xwyy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), w(), y(), y());
	}

	TVec4<T> xwyz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), w(), y(), z());
	}

	TVec4<T> xwyw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), w(), y(), w());
	}

	TVec4<T> xwzx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), w(), z(), x());
	}

	TVec4<T> xwzy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), w(), z(), y());
	}

	TVec4<T> xwzz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), w(), z(), z());
	}

	TVec4<T> xwzw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), w(), z(), w());
	}

	TVec4<T> xwwx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), w(), w(), x());
	}

	TVec4<T> xwwy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), w(), w(), y());
	}

	TVec4<T> xwwz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), w(), w(), z());
	}

	TVec4<T> xwww() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(x(), w(), w(), w());
	}

	TVec4<T> yxxx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), x(), x(), x());
	}

	TVec4<T> yxxy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), x(), x(), y());
	}

	TVec4<T> yxxz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), x(), x(), z());
	}

	TVec4<T> yxxw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), x(), x(), w());
	}

	TVec4<T> yxyx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), x(), y(), x());
	}

	TVec4<T> yxyy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), x(), y(), y());
	}

	TVec4<T> yxyz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), x(), y(), z());
	}

	TVec4<T> yxyw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), x(), y(), w());
	}

	TVec4<T> yxzx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), x(), z(), x());
	}

	TVec4<T> yxzy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), x(), z(), y());
	}

	TVec4<T> yxzz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), x(), z(), z());
	}

	TVec4<T> yxzw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), x(), z(), w());
	}

	TVec4<T> yxwx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), x(), w(), x());
	}

	TVec4<T> yxwy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), x(), w(), y());
	}

	TVec4<T> yxwz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), x(), w(), z());
	}

	TVec4<T> yxww() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), x(), w(), w());
	}

	TVec4<T> yyxx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), y(), x(), x());
	}

	TVec4<T> yyxy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), y(), x(), y());
	}

	TVec4<T> yyxz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), y(), x(), z());
	}

	TVec4<T> yyxw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), y(), x(), w());
	}

	TVec4<T> yyyx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), y(), y(), x());
	}

	TVec4<T> yyyy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), y(), y(), y());
	}

	TVec4<T> yyyz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), y(), y(), z());
	}

	TVec4<T> yyyw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), y(), y(), w());
	}

	TVec4<T> yyzx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), y(), z(), x());
	}

	TVec4<T> yyzy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), y(), z(), y());
	}

	TVec4<T> yyzz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), y(), z(), z());
	}

	TVec4<T> yyzw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), y(), z(), w());
	}

	TVec4<T> yywx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), y(), w(), x());
	}

	TVec4<T> yywy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), y(), w(), y());
	}

	TVec4<T> yywz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), y(), w(), z());
	}

	TVec4<T> yyww() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), y(), w(), w());
	}

	TVec4<T> yzxx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), z(), x(), x());
	}

	TVec4<T> yzxy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), z(), x(), y());
	}

	TVec4<T> yzxz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), z(), x(), z());
	}

	TVec4<T> yzxw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), z(), x(), w());
	}

	TVec4<T> yzyx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), z(), y(), x());
	}

	TVec4<T> yzyy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), z(), y(), y());
	}

	TVec4<T> yzyz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), z(), y(), z());
	}

	TVec4<T> yzyw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), z(), y(), w());
	}

	TVec4<T> yzzx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), z(), z(), x());
	}

	TVec4<T> yzzy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), z(), z(), y());
	}

	TVec4<T> yzzz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), z(), z(), z());
	}

	TVec4<T> yzzw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), z(), z(), w());
	}

	TVec4<T> yzwx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), z(), w(), x());
	}

	TVec4<T> yzwy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), z(), w(), y());
	}

	TVec4<T> yzwz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), z(), w(), z());
	}

	TVec4<T> yzww() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), z(), w(), w());
	}

	TVec4<T> ywxx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), w(), x(), x());
	}

	TVec4<T> ywxy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), w(), x(), y());
	}

	TVec4<T> ywxz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), w(), x(), z());
	}

	TVec4<T> ywxw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), w(), x(), w());
	}

	TVec4<T> ywyx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), w(), y(), x());
	}

	TVec4<T> ywyy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), w(), y(), y());
	}

	TVec4<T> ywyz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), w(), y(), z());
	}

	TVec4<T> ywyw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), w(), y(), w());
	}

	TVec4<T> ywzx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), w(), z(), x());
	}

	TVec4<T> ywzy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), w(), z(), y());
	}

	TVec4<T> ywzz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), w(), z(), z());
	}

	TVec4<T> ywzw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), w(), z(), w());
	}

	TVec4<T> ywwx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), w(), w(), x());
	}

	TVec4<T> ywwy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), w(), w(), y());
	}

	TVec4<T> ywwz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), w(), w(), z());
	}

	TVec4<T> ywww() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(y(), w(), w(), w());
	}

	TVec4<T> zxxx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), x(), x(), x());
	}

	TVec4<T> zxxy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), x(), x(), y());
	}

	TVec4<T> zxxz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), x(), x(), z());
	}

	TVec4<T> zxxw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), x(), x(), w());
	}

	TVec4<T> zxyx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), x(), y(), x());
	}

	TVec4<T> zxyy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), x(), y(), y());
	}

	TVec4<T> zxyz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), x(), y(), z());
	}

	TVec4<T> zxyw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), x(), y(), w());
	}

	TVec4<T> zxzx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), x(), z(), x());
	}

	TVec4<T> zxzy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), x(), z(), y());
	}

	TVec4<T> zxzz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), x(), z(), z());
	}

	TVec4<T> zxzw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), x(), z(), w());
	}

	TVec4<T> zxwx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), x(), w(), x());
	}

	TVec4<T> zxwy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), x(), w(), y());
	}

	TVec4<T> zxwz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), x(), w(), z());
	}

	TVec4<T> zxww() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), x(), w(), w());
	}

	TVec4<T> zyxx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), y(), x(), x());
	}

	TVec4<T> zyxy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), y(), x(), y());
	}

	TVec4<T> zyxz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), y(), x(), z());
	}

	TVec4<T> zyxw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), y(), x(), w());
	}

	TVec4<T> zyyx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), y(), y(), x());
	}

	TVec4<T> zyyy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), y(), y(), y());
	}

	TVec4<T> zyyz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), y(), y(), z());
	}

	TVec4<T> zyyw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), y(), y(), w());
	}

	TVec4<T> zyzx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), y(), z(), x());
	}

	TVec4<T> zyzy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), y(), z(), y());
	}

	TVec4<T> zyzz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), y(), z(), z());
	}

	TVec4<T> zyzw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), y(), z(), w());
	}

	TVec4<T> zywx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), y(), w(), x());
	}

	TVec4<T> zywy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), y(), w(), y());
	}

	TVec4<T> zywz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), y(), w(), z());
	}

	TVec4<T> zyww() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), y(), w(), w());
	}

	TVec4<T> zzxx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), z(), x(), x());
	}

	TVec4<T> zzxy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), z(), x(), y());
	}

	TVec4<T> zzxz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), z(), x(), z());
	}

	TVec4<T> zzxw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), z(), x(), w());
	}

	TVec4<T> zzyx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), z(), y(), x());
	}

	TVec4<T> zzyy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), z(), y(), y());
	}

	TVec4<T> zzyz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), z(), y(), z());
	}

	TVec4<T> zzyw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), z(), y(), w());
	}

	TVec4<T> zzzx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), z(), z(), x());
	}

	TVec4<T> zzzy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), z(), z(), y());
	}

	TVec4<T> zzzz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), z(), z(), z());
	}

	TVec4<T> zzzw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), z(), z(), w());
	}

	TVec4<T> zzwx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), z(), w(), x());
	}

	TVec4<T> zzwy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), z(), w(), y());
	}

	TVec4<T> zzwz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), z(), w(), z());
	}

	TVec4<T> zzww() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), z(), w(), w());
	}

	TVec4<T> zwxx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), w(), x(), x());
	}

	TVec4<T> zwxy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), w(), x(), y());
	}

	TVec4<T> zwxz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), w(), x(), z());
	}

	TVec4<T> zwxw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), w(), x(), w());
	}

	TVec4<T> zwyx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), w(), y(), x());
	}

	TVec4<T> zwyy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), w(), y(), y());
	}

	TVec4<T> zwyz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), w(), y(), z());
	}

	TVec4<T> zwyw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), w(), y(), w());
	}

	TVec4<T> zwzx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), w(), z(), x());
	}

	TVec4<T> zwzy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), w(), z(), y());
	}

	TVec4<T> zwzz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), w(), z(), z());
	}

	TVec4<T> zwzw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), w(), z(), w());
	}

	TVec4<T> zwwx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), w(), w(), x());
	}

	TVec4<T> zwwy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), w(), w(), y());
	}

	TVec4<T> zwwz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), w(), w(), z());
	}

	TVec4<T> zwww() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(z(), w(), w(), w());
	}

	TVec4<T> wxxx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), x(), x(), x());
	}

	TVec4<T> wxxy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), x(), x(), y());
	}

	TVec4<T> wxxz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), x(), x(), z());
	}

	TVec4<T> wxxw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), x(), x(), w());
	}

	TVec4<T> wxyx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), x(), y(), x());
	}

	TVec4<T> wxyy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), x(), y(), y());
	}

	TVec4<T> wxyz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), x(), y(), z());
	}

	TVec4<T> wxyw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), x(), y(), w());
	}

	TVec4<T> wxzx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), x(), z(), x());
	}

	TVec4<T> wxzy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), x(), z(), y());
	}

	TVec4<T> wxzz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), x(), z(), z());
	}

	TVec4<T> wxzw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), x(), z(), w());
	}

	TVec4<T> wxwx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), x(), w(), x());
	}

	TVec4<T> wxwy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), x(), w(), y());
	}

	TVec4<T> wxwz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), x(), w(), z());
	}

	TVec4<T> wxww() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), x(), w(), w());
	}

	TVec4<T> wyxx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), y(), x(), x());
	}

	TVec4<T> wyxy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), y(), x(), y());
	}

	TVec4<T> wyxz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), y(), x(), z());
	}

	TVec4<T> wyxw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), y(), x(), w());
	}

	TVec4<T> wyyx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), y(), y(), x());
	}

	TVec4<T> wyyy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), y(), y(), y());
	}

	TVec4<T> wyyz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), y(), y(), z());
	}

	TVec4<T> wyyw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), y(), y(), w());
	}

	TVec4<T> wyzx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), y(), z(), x());
	}

	TVec4<T> wyzy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), y(), z(), y());
	}

	TVec4<T> wyzz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), y(), z(), z());
	}

	TVec4<T> wyzw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), y(), z(), w());
	}

	TVec4<T> wywx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), y(), w(), x());
	}

	TVec4<T> wywy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), y(), w(), y());
	}

	TVec4<T> wywz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), y(), w(), z());
	}

	TVec4<T> wyww() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), y(), w(), w());
	}

	TVec4<T> wzxx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), z(), x(), x());
	}

	TVec4<T> wzxy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), z(), x(), y());
	}

	TVec4<T> wzxz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), z(), x(), z());
	}

	TVec4<T> wzxw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), z(), x(), w());
	}

	TVec4<T> wzyx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), z(), y(), x());
	}

	TVec4<T> wzyy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), z(), y(), y());
	}

	TVec4<T> wzyz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), z(), y(), z());
	}

	TVec4<T> wzyw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), z(), y(), w());
	}

	TVec4<T> wzzx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), z(), z(), x());
	}

	TVec4<T> wzzy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), z(), z(), y());
	}

	TVec4<T> wzzz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), z(), z(), z());
	}

	TVec4<T> wzzw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), z(), z(), w());
	}

	TVec4<T> wzwx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), z(), w(), x());
	}

	TVec4<T> wzwy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), z(), w(), y());
	}

	TVec4<T> wzwz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), z(), w(), z());
	}

	TVec4<T> wzww() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), z(), w(), w());
	}

	TVec4<T> wwxx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), w(), x(), x());
	}

	TVec4<T> wwxy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), w(), x(), y());
	}

	TVec4<T> wwxz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), w(), x(), z());
	}

	TVec4<T> wwxw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), w(), x(), w());
	}

	TVec4<T> wwyx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), w(), y(), x());
	}

	TVec4<T> wwyy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), w(), y(), y());
	}

	TVec4<T> wwyz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), w(), y(), z());
	}

	TVec4<T> wwyw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), w(), y(), w());
	}

	TVec4<T> wwzx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), w(), z(), x());
	}

	TVec4<T> wwzy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), w(), z(), y());
	}

	TVec4<T> wwzz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), w(), z(), z());
	}

	TVec4<T> wwzw() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), w(), z(), w());
	}

	TVec4<T> wwwx() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), w(), w(), x());
	}

	TVec4<T> wwwy() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), w(), w(), y());
	}

	TVec4<T> wwwz() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), w(), w(), z());
	}

	TVec4<T> wwww() const
	{
		static_assert(N > 3, "Wrong vector");
		return TVec4<T>(w(), w(), w(), w());
	}

	TVec4<T> xyz1() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec4<T>(x(), y(), z(), static_cast<T>(1));
	}

	TVec4<T> xyz0() const
	{
		static_assert(N > 2, "Wrong vector");
		return TVec4<T>(x(), y(), z(), static_cast<T>(0));
	}

	T& operator[](const U i)
	{
		return m_arr[i];
	}

	const T& operator[](const U i) const
	{
		return m_arr[i];
	}

	TSimd& getSimd()
	{
		return m_simd;
	}

	const TSimd& getSimd() const
	{
		return m_simd;
	}
	/// @}

	/// @name Operators with same type
	/// @{
	TVec& operator=(const TVec& b)
	{
		for(U i = 0; i < N; i++)
		{
			m_arr[i] = b.m_arr[i];
		}
		return *this;
	}

	TV& operator=(const TV& b)
	{
		for(U i = 0; i < N; i++)
		{
			m_arr[i] = b.m_arr[i];
		}
		return static_cast<TV&>(*this);
	}

	TV operator+(const TV& b) const
	{
		TV out;
		for(U i = 0; i < N; i++)
		{
			out.m_arr[i] = m_arr[i] + b.m_arr[i];
		}
		return out;
	}

	TV& operator+=(const TV& b)
	{
		for(U i = 0; i < N; i++)
		{
			m_arr[i] += b.m_arr[i];
		}
		return static_cast<TV&>(*this);
	}

	TV operator-(const TV& b) const
	{
		TV out;
		for(U i = 0; i < N; i++)
		{
			out.m_arr[i] = m_arr[i] - b.m_arr[i];
		}
		return out;
	}

	TV& operator-=(const TV& b)
	{
		for(U i = 0; i < N; i++)
		{
			m_arr[i] -= b.m_arr[i];
		}
		return static_cast<TV&>(*this);
	}

	TV operator*(const TV& b) const
	{
		TV out;
		for(U i = 0; i < N; i++)
		{
			out.m_arr[i] = m_arr[i] * b.m_arr[i];
		}
		return out;
	}

	TV& operator*=(const TV& b)
	{
		for(U i = 0; i < N; i++)
		{
			m_arr[i] *= b.m_arr[i];
		}
		return static_cast<TV&>(*this);
	}

	TV operator/(const TV& b) const
	{
		TV out;
		for(U i = 0; i < N; i++)
		{
			ANKI_ASSERT(b.m_arr[i] != 0.0);
			out.m_arr[i] = m_arr[i] / b.m_arr[i];
		}
		return out;
	}

	TV& operator/=(const TV& b)
	{
		for(U i = 0; i < N; i++)
		{
			ANKI_ASSERT(b.m_arr[i] != 0.0);
			m_arr[i] /= b.m_arr[i];
		}
		return static_cast<TV&>(*this);
	}

	TV operator-() const
	{
		TV out;
		for(U i = 0; i < N; i++)
		{
			out.m_arr[i] = -m_arr[i];
		}
		return out;
	}

	Bool operator==(const TV& b) const
	{
		for(U i = 0; i < N; i++)
		{
			if(!isZero<T>(m_arr[i] - b.m_arr[i]))
			{
				return false;
			}
		}
		return true;
	}

	Bool operator!=(const TV& b) const
	{
		return !operator==(b);
	}

	Bool operator<(const TV& b) const
	{
		for(U i = 0; i < N; i++)
		{
			if(m_arr[i] >= b.m_arr[i])
			{
				return false;
			}
		}
		return true;
	}

	Bool operator<=(const TV& b) const
	{
		for(U i = 0; i < N; i++)
		{
			if(m_arr[i] > b.m_arr[i])
			{
				return false;
			}
		}
		return true;
	}

	Bool operator>(const TV& b) const
	{
		for(U i = 0; i < N; i++)
		{
			if(m_arr[i] <= b.m_arr[i])
			{
				return false;
			}
		}
		return true;
	}

	Bool operator>=(const TV& b) const
	{
		for(U i = 0; i < N; i++)
		{
			if(m_arr[i] < b.m_arr[i])
			{
				return false;
			}
		}
		return true;
	}
	/// @}

	/// @name Operators with T
	/// @{
	TV operator+(const T f) const
	{
		return (*this) + TV(f);
	}

	TV& operator+=(const T f)
	{
		(*this) += TV(f);
		return static_cast<TV&>(*this);
	}

	TV operator-(const T f) const
	{
		return (*this) - TV(f);
	}

	TV& operator-=(const T f)
	{
		(*this) -= TV(f);
		return static_cast<TV&>(*this);
	}

	TV operator*(const T f) const
	{
		return (*this) * TV(f);
	}

	TV& operator*=(const T f)
	{
		(*this) *= TV(f);
		return static_cast<TV&>(*this);
	}

	TV operator/(const T f) const
	{
		return (*this) / TV(f);
	}

	TV& operator/=(const T f)
	{
		(*this) /= TV(f);
		return static_cast<TV&>(*this);
	}
	/// @}

	/// @name Other
	/// @{
	T dot(const TV& b) const
	{
		T out = 0.0;
		for(U i = 0; i < N; i++)
		{
			out += m_arr[i] * b.m_arr[i];
		}
		return out;
	}

	T getLengthSquared() const
	{
		T out = 0.0;
		for(U i = 0; i < N; i++)
		{
			out += m_arr[i] * m_arr[i];
		}
		return out;
	}

	T getLength() const
	{
		return sqrt<T>(getLengthSquared());
	}

	T getDistanceSquared(const TV& b) const
	{
		return ((*this) - b).getLengthSquared();
	}

	T getDistance(const TV& b) const
	{
		return sqrt<T>(getDistance(b));
	}

	void normalize()
	{
		(*this) /= getLength();
	}

	TV getNormalized() const
	{
		return (*this) / getLength();
	}

	/// Return lerp(this, v1, t)
	TV lerp(const TV& v1, T t) const
	{
		return ((*this) * (1.0 - t)) + (v1 * t);
	}

	TV getAbs() const
	{
		TV out;
		for(U i = 0; i < N; ++i)
		{
			out[i] = absolute<T>(m_arr[i]);
		}
		return out;
	}

	/// Clamp between two values.
	void clamp(const T& minv, const T& maxv)
	{
		*this = max(TV(minv)).min(TV(maxv));
	}

	/// Get clamped between two values.
	TV getClamped(const T& minv, const T& maxv) const
	{
		return max(TV(minv)).min(TV(maxv));
	}

	/// Clamp between two vectors.
	void clamp(const TV& minv, const TV& maxv)
	{
		*this = max(minv).min(maxv);
	}

	/// Get clamped between two vectors.
	TV getClamped(const TV& minv, const TV& maxv) const
	{
		return max(minv).min(maxv);
	}

	/// Get the min of all components.
	TV min(const T& b) const
	{
		TV out;
		for(U i = 0; i < N; ++i)
		{
			out[i] = anki::min<T>(m_arr[i], b);
		}
		return out;
	}

	/// Get the min of all components.
	TV min(const TV& b) const
	{
		TV out;
		for(U i = 0; i < N; ++i)
		{
			out[i] = anki::min<T>(m_arr[i], b[i]);
		}
		return out;
	}

	/// Get the max of all components.
	TV max(const TV& b) const
	{
		TV out;
		for(U i = 0; i < N; ++i)
		{
			out[i] = anki::max<T>(m_arr[i], b[i]);
		}
		return out;
	}

	/// Get the max of all components.
	TV max(const T& b) const
	{
		TV out;
		for(U i = 0; i < N; ++i)
		{
			out[i] = anki::max<T>(m_arr[i], b);
		}
		return out;
	}

	/// Serialize the structure.
	void serialize(void* data, PtrSize& size) const
	{
		size = sizeof(*this);
		if(data)
		{
			memcpy(data, this, sizeof(*this));
		}
	}

	/// De-serialize the structure.
	void deserialize(const void* data)
	{
		ANKI_ASSERT(data);
		memcpy(this, data, sizeof(*this));
	}

	template<typename TAlloc>
	String toString(TAlloc alloc) const
	{
		ANKI_ASSERT(0 && "TODO");
		return String();
	}
	/// @}

protected:
	/// @name Data
	/// @{
	union
	{
		Array<T, N> m_arr;
		TSimd m_simd;
	};
	/// @}
};
/// @}

} // end namespace anki
