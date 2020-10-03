// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/math/Common.h>
#include <anki/util/F16.h>

namespace anki
{

/// @addtogroup math
/// @{

/// Common code for all vectors
/// @tparam T The scalar type. Eg float.
/// @tparam N The number of the vector components (2, 3 or 4).
template<typename T, U N>
class alignas(MathSimd<T, N>::ALIGNMENT) TVec
{
public:
	using Scalar = T;
	using Simd = typename MathSimd<T, N>::Type;
	static constexpr U COMPONENT_COUNT = N;
	static constexpr Bool IS_INTEGER = std::is_integral<T>::value;
	static constexpr Bool HAS_VEC4_SIMD = N == 4 && std::is_same<T, F32>::value && ANKI_SIMD_SSE;

	/// @name Constructors
	/// @{

	/// Defaut constructor. IT WILL NOT INITIALIZE ANYTHING.
	TVec()
	{
	}

	/// Copy.
	TVec(ANKI_ENABLE_ARG(const TVec&, !HAS_VEC4_SIMD) b)
	{
		for(U i = 0; i < N; i++)
		{
			m_arr[i] = b.m_arr[i];
		}
	}

	/// Copy.
	TVec(ANKI_ENABLE_ARG(const TVec&, HAS_VEC4_SIMD) b)
	{
		m_simd = b.m_simd;
	}

	/// Convert from another type.
	template<typename Y, ANKI_ENABLE(!std::is_same<Y, T>::value)>
	explicit TVec(const TVec<Y, N>& b)
	{
		for(U i = 0; i < N; i++)
		{
			m_arr[i] = T(b[i]);
		}
	}

	ANKI_ENABLE_METHOD(!HAS_VEC4_SIMD)
	explicit TVec(const T f)
	{
		for(U i = 0; i < N; ++i)
		{
			m_arr[i] = f;
		}
	}

	ANKI_ENABLE_METHOD(HAS_VEC4_SIMD)
	explicit TVec(const T f)
	{
		m_simd = _mm_set1_ps(f);
	}

	ANKI_ENABLE_METHOD(!HAS_VEC4_SIMD)
	explicit TVec(const T arr[])
	{
		for(U i = 0; i < N; ++i)
		{
			m_arr[i] = arr[i];
		}
	}

	ANKI_ENABLE_METHOD(HAS_VEC4_SIMD)
	explicit TVec(const T arr[])
	{
		m_simd = _mm_load_ps(arr);
	}

	explicit TVec(const Simd& simd)
	{
		m_simd = simd;
	}

	ANKI_ENABLE_METHOD(N == 2)
	TVec(const T x_, const T y_)
	{
		x() = x_;
		y() = y_;
	}

	// Vec3 specific

	ANKI_ENABLE_METHOD(N == 3)
	TVec(const T x_, const T y_, const T z_)
	{
		x() = x_;
		y() = y_;
		z() = z_;
	}

	ANKI_ENABLE_METHOD(N == 3)
	TVec(const TVec<T, 2>& a, const T z_)
	{
		x() = a.x();
		y() = a.y();
		z() = z_;
	}

	ANKI_ENABLE_METHOD(N == 3)
	TVec(const T x_, const TVec<T, 2>& a)
	{
		x() = x_;
		y() = a.x();
		z() = a.y();
	}

	// Vec4 specific

	ANKI_ENABLE_METHOD(N == 4 && !HAS_VEC4_SIMD)
	TVec(const T x_, const T y_, const T z_, const T w_)
	{
		x() = x_;
		y() = y_;
		z() = z_;
		w() = w_;
	}

	ANKI_ENABLE_METHOD(HAS_VEC4_SIMD)
	TVec(const T x_, const T y_, const T z_, const T w_)
	{
		m_simd = _mm_set_ps(w_, z_, y_, x_);
	}

	ANKI_ENABLE_METHOD(N == 4)
	TVec(const TVec<T, 3>& a, const T w_)
	{
		x() = a.x();
		y() = a.y();
		z() = a.z();
		w() = w_;
	}

	ANKI_ENABLE_METHOD(N == 4)
	TVec(const T x_, const TVec<T, 3>& a)
	{
		x() = x_;
		y() = a.x();
		z() = a.y();
		w() = a.z();
	}

	ANKI_ENABLE_METHOD(N == 4)
	TVec(const TVec<T, 2>& a, const T z_, const T w_)
	{
		x() = a.x();
		y() = a.y();
		z() = z_;
		w() = w_;
	}

	ANKI_ENABLE_METHOD(N == 4)
	TVec(const T x_, const TVec<T, 2>& a, const T w_)
	{
		x() = x_;
		y() = a.x();
		z() = a.y();
		w() = w_;
	}

	ANKI_ENABLE_METHOD(N == 4)
	TVec(const T x_, const T y_, const TVec<T, 2>& a)
	{
		x() = x_;
		y() = y_;
		z() = a.x();
		w() = a.y();
	}

	ANKI_ENABLE_METHOD(N == 4)
	TVec(const TVec<T, 2>& a, const TVec<T, 2>& b)
	{
		x() = a.x();
		y() = a.y();
		z() = b.x();
		w() = b.y();
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

	ANKI_ENABLE_METHOD(N > 2)
	T& z()
	{
		return m_arr[2];
	}

	ANKI_ENABLE_METHOD(N > 2)
	T z() const
	{
		return m_arr[2];
	}

	ANKI_ENABLE_METHOD(N > 3)
	T& w()
	{
		return m_arr[3];
	}

	ANKI_ENABLE_METHOD(N > 3)
	T w() const
	{
		return m_arr[3];
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> xyz1() const
	{
		return TVec<T, 4>(x(), y(), z(), T(1));
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> xyz0() const
	{
		return TVec<T, 4>(x(), y(), z(), T(0));
	}

	T& operator[](const U i)
	{
		return m_arr[i];
	}

	const T& operator[](const U i) const
	{
		return m_arr[i];
	}

	Simd& getSimd()
	{
		return m_simd;
	}

	const Simd& getSimd() const
	{
		return m_simd;
	}

	// Swizzled accessors

	ANKI_ENABLE_METHOD(N > 0)
	TVec<T, 2> xx() const
	{
		return TVec<T, 2>(m_carr[0], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 1)
	TVec<T, 2> xy() const
	{
		return TVec<T, 2>(m_carr[0], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 2> xz() const
	{
		return TVec<T, 2>(m_carr[0], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 2> xw() const
	{
		return TVec<T, 2>(m_carr[0], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 1)
	TVec<T, 2> yx() const
	{
		return TVec<T, 2>(m_carr[1], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 1)
	TVec<T, 2> yy() const
	{
		return TVec<T, 2>(m_carr[1], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 2> yz() const
	{
		return TVec<T, 2>(m_carr[1], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 2> yw() const
	{
		return TVec<T, 2>(m_carr[1], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 2> zx() const
	{
		return TVec<T, 2>(m_carr[2], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 2> zy() const
	{
		return TVec<T, 2>(m_carr[2], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 2> zz() const
	{
		return TVec<T, 2>(m_carr[2], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 2> zw() const
	{
		return TVec<T, 2>(m_carr[2], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 2> wx() const
	{
		return TVec<T, 2>(m_carr[3], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 2> wy() const
	{
		return TVec<T, 2>(m_carr[3], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 2> wz() const
	{
		return TVec<T, 2>(m_carr[3], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 2> ww() const
	{
		return TVec<T, 2>(m_carr[3], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 0)
	TVec<T, 3> xxx() const
	{
		return TVec<T, 3>(m_carr[0], m_carr[0], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 1)
	TVec<T, 3> xxy() const
	{
		return TVec<T, 3>(m_carr[0], m_carr[0], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 3> xxz() const
	{
		return TVec<T, 3>(m_carr[0], m_carr[0], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> xxw() const
	{
		return TVec<T, 3>(m_carr[0], m_carr[0], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 1)
	TVec<T, 3> xyx() const
	{
		return TVec<T, 3>(m_carr[0], m_carr[1], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 1)
	TVec<T, 3> xyy() const
	{
		return TVec<T, 3>(m_carr[0], m_carr[1], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 3> xyz() const
	{
		return TVec<T, 3>(m_carr[0], m_carr[1], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> xyw() const
	{
		return TVec<T, 3>(m_carr[0], m_carr[1], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 3> xzx() const
	{
		return TVec<T, 3>(m_carr[0], m_carr[2], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 3> xzy() const
	{
		return TVec<T, 3>(m_carr[0], m_carr[2], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 3> xzz() const
	{
		return TVec<T, 3>(m_carr[0], m_carr[2], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> xzw() const
	{
		return TVec<T, 3>(m_carr[0], m_carr[2], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> xwx() const
	{
		return TVec<T, 3>(m_carr[0], m_carr[3], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> xwy() const
	{
		return TVec<T, 3>(m_carr[0], m_carr[3], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> xwz() const
	{
		return TVec<T, 3>(m_carr[0], m_carr[3], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> xww() const
	{
		return TVec<T, 3>(m_carr[0], m_carr[3], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 1)
	TVec<T, 3> yxx() const
	{
		return TVec<T, 3>(m_carr[1], m_carr[0], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 1)
	TVec<T, 3> yxy() const
	{
		return TVec<T, 3>(m_carr[1], m_carr[0], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 3> yxz() const
	{
		return TVec<T, 3>(m_carr[1], m_carr[0], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> yxw() const
	{
		return TVec<T, 3>(m_carr[1], m_carr[0], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 1)
	TVec<T, 3> yyx() const
	{
		return TVec<T, 3>(m_carr[1], m_carr[1], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 1)
	TVec<T, 3> yyy() const
	{
		return TVec<T, 3>(m_carr[1], m_carr[1], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 3> yyz() const
	{
		return TVec<T, 3>(m_carr[1], m_carr[1], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> yyw() const
	{
		return TVec<T, 3>(m_carr[1], m_carr[1], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 3> yzx() const
	{
		return TVec<T, 3>(m_carr[1], m_carr[2], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 3> yzy() const
	{
		return TVec<T, 3>(m_carr[1], m_carr[2], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 3> yzz() const
	{
		return TVec<T, 3>(m_carr[1], m_carr[2], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> yzw() const
	{
		return TVec<T, 3>(m_carr[1], m_carr[2], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> ywx() const
	{
		return TVec<T, 3>(m_carr[1], m_carr[3], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> ywy() const
	{
		return TVec<T, 3>(m_carr[1], m_carr[3], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> ywz() const
	{
		return TVec<T, 3>(m_carr[1], m_carr[3], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> yww() const
	{
		return TVec<T, 3>(m_carr[1], m_carr[3], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 3> zxx() const
	{
		return TVec<T, 3>(m_carr[2], m_carr[0], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 3> zxy() const
	{
		return TVec<T, 3>(m_carr[2], m_carr[0], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 3> zxz() const
	{
		return TVec<T, 3>(m_carr[2], m_carr[0], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> zxw() const
	{
		return TVec<T, 3>(m_carr[2], m_carr[0], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 3> zyx() const
	{
		return TVec<T, 3>(m_carr[2], m_carr[1], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 3> zyy() const
	{
		return TVec<T, 3>(m_carr[2], m_carr[1], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 3> zyz() const
	{
		return TVec<T, 3>(m_carr[2], m_carr[1], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> zyw() const
	{
		return TVec<T, 3>(m_carr[2], m_carr[1], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 3> zzx() const
	{
		return TVec<T, 3>(m_carr[2], m_carr[2], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 3> zzy() const
	{
		return TVec<T, 3>(m_carr[2], m_carr[2], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 3> zzz() const
	{
		return TVec<T, 3>(m_carr[2], m_carr[2], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> zzw() const
	{
		return TVec<T, 3>(m_carr[2], m_carr[2], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> zwx() const
	{
		return TVec<T, 3>(m_carr[2], m_carr[3], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> zwy() const
	{
		return TVec<T, 3>(m_carr[2], m_carr[3], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> zwz() const
	{
		return TVec<T, 3>(m_carr[2], m_carr[3], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> zww() const
	{
		return TVec<T, 3>(m_carr[2], m_carr[3], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> wxx() const
	{
		return TVec<T, 3>(m_carr[3], m_carr[0], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> wxy() const
	{
		return TVec<T, 3>(m_carr[3], m_carr[0], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> wxz() const
	{
		return TVec<T, 3>(m_carr[3], m_carr[0], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> wxw() const
	{
		return TVec<T, 3>(m_carr[3], m_carr[0], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> wyx() const
	{
		return TVec<T, 3>(m_carr[3], m_carr[1], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> wyy() const
	{
		return TVec<T, 3>(m_carr[3], m_carr[1], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> wyz() const
	{
		return TVec<T, 3>(m_carr[3], m_carr[1], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> wyw() const
	{
		return TVec<T, 3>(m_carr[3], m_carr[1], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> wzx() const
	{
		return TVec<T, 3>(m_carr[3], m_carr[2], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> wzy() const
	{
		return TVec<T, 3>(m_carr[3], m_carr[2], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> wzz() const
	{
		return TVec<T, 3>(m_carr[3], m_carr[2], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> wzw() const
	{
		return TVec<T, 3>(m_carr[3], m_carr[2], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> wwx() const
	{
		return TVec<T, 3>(m_carr[3], m_carr[3], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> wwy() const
	{
		return TVec<T, 3>(m_carr[3], m_carr[3], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> wwz() const
	{
		return TVec<T, 3>(m_carr[3], m_carr[3], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 3> www() const
	{
		return TVec<T, 3>(m_carr[3], m_carr[3], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 0)
	TVec<T, 4> xxxx() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[0], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 1)
	TVec<T, 4> xxxy() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[0], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> xxxz() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[0], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xxxw() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[0], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 1)
	TVec<T, 4> xxyx() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[1], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 1)
	TVec<T, 4> xxyy() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[1], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> xxyz() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[1], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xxyw() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[1], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> xxzx() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[2], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> xxzy() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[2], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> xxzz() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[2], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xxzw() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[2], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xxwx() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[3], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xxwy() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[3], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xxwz() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[3], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xxww() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[3], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 1)
	TVec<T, 4> xyxx() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[0], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 1)
	TVec<T, 4> xyxy() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[0], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> xyxz() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[0], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xyxw() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[0], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 1)
	TVec<T, 4> xyyx() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[1], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 1)
	TVec<T, 4> xyyy() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[1], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> xyyz() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[1], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xyyw() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[1], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> xyzx() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[2], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> xyzy() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[2], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> xyzz() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[2], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xyzw() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[2], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xywx() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[3], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xywy() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[3], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xywz() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[3], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xyww() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[3], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> xzxx() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[0], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> xzxy() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[0], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> xzxz() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[0], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xzxw() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[0], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> xzyx() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[1], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> xzyy() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[1], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> xzyz() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[1], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xzyw() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[1], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> xzzx() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[2], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> xzzy() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[2], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> xzzz() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[2], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xzzw() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[2], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xzwx() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[3], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xzwy() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[3], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xzwz() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[3], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xzww() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[3], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xwxx() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[0], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xwxy() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[0], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xwxz() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[0], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xwxw() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[0], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xwyx() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[1], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xwyy() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[1], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xwyz() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[1], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xwyw() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[1], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xwzx() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[2], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xwzy() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[2], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xwzz() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[2], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xwzw() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[2], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xwwx() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[3], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xwwy() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[3], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xwwz() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[3], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> xwww() const
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[3], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 1)
	TVec<T, 4> yxxx() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[0], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 1)
	TVec<T, 4> yxxy() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[0], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> yxxz() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[0], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> yxxw() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[0], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 1)
	TVec<T, 4> yxyx() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[1], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 1)
	TVec<T, 4> yxyy() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[1], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> yxyz() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[1], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> yxyw() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[1], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> yxzx() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[2], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> yxzy() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[2], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> yxzz() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[2], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> yxzw() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[2], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> yxwx() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[3], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> yxwy() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[3], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> yxwz() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[3], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> yxww() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[3], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 1)
	TVec<T, 4> yyxx() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[0], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 1)
	TVec<T, 4> yyxy() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[0], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> yyxz() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[0], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> yyxw() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[0], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 1)
	TVec<T, 4> yyyx() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[1], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 1)
	TVec<T, 4> yyyy() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[1], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> yyyz() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[1], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> yyyw() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[1], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> yyzx() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[2], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> yyzy() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[2], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> yyzz() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[2], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> yyzw() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[2], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> yywx() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[3], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> yywy() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[3], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> yywz() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[3], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> yyww() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[3], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> yzxx() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[0], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> yzxy() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[0], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> yzxz() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[0], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> yzxw() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[0], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> yzyx() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[1], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> yzyy() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[1], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> yzyz() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[1], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> yzyw() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[1], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> yzzx() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[2], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> yzzy() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[2], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> yzzz() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[2], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> yzzw() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[2], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> yzwx() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[3], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> yzwy() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[3], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> yzwz() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[3], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> yzww() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[3], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> ywxx() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[0], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> ywxy() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[0], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> ywxz() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[0], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> ywxw() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[0], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> ywyx() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[1], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> ywyy() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[1], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> ywyz() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[1], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> ywyw() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[1], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> ywzx() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[2], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> ywzy() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[2], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> ywzz() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[2], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> ywzw() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[2], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> ywwx() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[3], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> ywwy() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[3], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> ywwz() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[3], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> ywww() const
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[3], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zxxx() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[0], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zxxy() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[0], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zxxz() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[0], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zxxw() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[0], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zxyx() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[1], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zxyy() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[1], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zxyz() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[1], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zxyw() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[1], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zxzx() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[2], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zxzy() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[2], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zxzz() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[2], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zxzw() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[2], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zxwx() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[3], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zxwy() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[3], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zxwz() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[3], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zxww() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[3], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zyxx() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[0], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zyxy() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[0], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zyxz() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[0], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zyxw() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[0], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zyyx() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[1], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zyyy() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[1], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zyyz() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[1], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zyyw() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[1], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zyzx() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[2], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zyzy() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[2], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zyzz() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[2], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zyzw() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[2], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zywx() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[3], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zywy() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[3], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zywz() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[3], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zyww() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[3], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zzxx() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[0], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zzxy() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[0], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zzxz() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[0], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zzxw() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[0], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zzyx() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[1], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zzyy() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[1], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zzyz() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[1], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zzyw() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[1], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zzzx() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[2], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zzzy() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[2], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 2)
	TVec<T, 4> zzzz() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[2], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zzzw() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[2], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zzwx() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[3], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zzwy() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[3], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zzwz() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[3], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zzww() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[3], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zwxx() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[0], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zwxy() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[0], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zwxz() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[0], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zwxw() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[0], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zwyx() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[1], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zwyy() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[1], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zwyz() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[1], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zwyw() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[1], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zwzx() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[2], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zwzy() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[2], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zwzz() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[2], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zwzw() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[2], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zwwx() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[3], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zwwy() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[3], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zwwz() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[3], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> zwww() const
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[3], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wxxx() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[0], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wxxy() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[0], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wxxz() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[0], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wxxw() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[0], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wxyx() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[1], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wxyy() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[1], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wxyz() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[1], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wxyw() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[1], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wxzx() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[2], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wxzy() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[2], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wxzz() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[2], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wxzw() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[2], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wxwx() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[3], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wxwy() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[3], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wxwz() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[3], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wxww() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[3], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wyxx() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[0], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wyxy() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[0], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wyxz() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[0], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wyxw() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[0], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wyyx() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[1], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wyyy() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[1], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wyyz() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[1], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wyyw() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[1], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wyzx() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[2], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wyzy() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[2], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wyzz() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[2], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wyzw() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[2], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wywx() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[3], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wywy() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[3], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wywz() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[3], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wyww() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[3], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wzxx() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[0], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wzxy() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[0], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wzxz() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[0], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wzxw() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[0], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wzyx() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[1], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wzyy() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[1], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wzyz() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[1], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wzyw() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[1], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wzzx() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[2], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wzzy() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[2], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wzzz() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[2], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wzzw() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[2], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wzwx() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[3], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wzwy() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[3], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wzwz() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[3], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wzww() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[3], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wwxx() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[0], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wwxy() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[0], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wwxz() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[0], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wwxw() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[0], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wwyx() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[1], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wwyy() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[1], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wwyz() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[1], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wwyw() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[1], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wwzx() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[2], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wwzy() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[2], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wwzz() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[2], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wwzw() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[2], m_carr[3]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wwwx() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[3], m_carr[0]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wwwy() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[3], m_carr[1]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wwwz() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[3], m_carr[2]);
	}

	ANKI_ENABLE_METHOD(N > 3)
	TVec<T, 4> wwww() const
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[3], m_carr[3]);
	}
	/// @}

	/// @name Operators with same type
	/// @{

	// Copy
	TVec& operator=(ANKI_ENABLE_ARG(const TVec&, !HAS_VEC4_SIMD) b)
	{
		for(U i = 0; i < N; i++)
		{
			m_arr[i] = b.m_carr[i];
		}
		return *this;
	}

	// Copy
	TVec& operator=(ANKI_ENABLE_ARG(const TVec&, HAS_VEC4_SIMD) b)
	{
		m_simd = b.m_simd;
		return *this;
	}

	ANKI_ENABLE_METHOD(!HAS_VEC4_SIMD)
	TVec operator+(const TVec& b) const
	{
		TVec out;
		for(U i = 0; i < N; i++)
		{
			out.m_arr[i] = m_arr[i] + b.m_arr[i];
		}
		return out;
	}

	ANKI_ENABLE_METHOD(HAS_VEC4_SIMD)
	TVec operator+(const TVec& b) const
	{
		return TVec(_mm_add_ps(m_simd, b.m_simd));
	}

	ANKI_ENABLE_METHOD(!HAS_VEC4_SIMD)
	TVec& operator+=(const TVec& b)
	{
		for(U i = 0; i < N; i++)
		{
			m_arr[i] += b.m_arr[i];
		}
		return *this;
	}

	ANKI_ENABLE_METHOD(HAS_VEC4_SIMD)
	TVec& operator+=(const TVec& b)
	{
		m_simd = _mm_add_ps(m_simd, b.m_simd);
		return *this;
	}

	ANKI_ENABLE_METHOD(!HAS_VEC4_SIMD)
	TVec operator-(const TVec& b) const
	{
		TVec out;
		for(U i = 0; i < N; i++)
		{
			out.m_arr[i] = m_arr[i] - b.m_arr[i];
		}
		return out;
	}

	ANKI_ENABLE_METHOD(HAS_VEC4_SIMD)
	TVec operator-(const TVec& b) const
	{
		return TVec(_mm_sub_ps(m_simd, b.m_simd));
	}

	ANKI_ENABLE_METHOD(!HAS_VEC4_SIMD)
	TVec& operator-=(const TVec& b)
	{
		for(U i = 0; i < N; i++)
		{
			m_arr[i] -= b.m_arr[i];
		}
		return *this;
	}

	ANKI_ENABLE_METHOD(HAS_VEC4_SIMD)
	TVec& operator-=(const TVec& b)
	{
		m_simd = _mm_sub_ps(m_simd, b.m_simd);
		return *this;
	}

	ANKI_ENABLE_METHOD(!HAS_VEC4_SIMD)
	TVec operator*(const TVec& b) const
	{
		TVec out;
		for(U i = 0; i < N; i++)
		{
			out.m_arr[i] = m_arr[i] * b.m_arr[i];
		}
		return out;
	}

	ANKI_ENABLE_METHOD(HAS_VEC4_SIMD)
	TVec operator*(const TVec& b) const
	{
		return TVec(_mm_mul_ps(m_simd, b.m_simd));
	}

	ANKI_ENABLE_METHOD(!HAS_VEC4_SIMD)
	TVec& operator*=(const TVec& b)
	{
		for(U i = 0; i < N; i++)
		{
			m_arr[i] *= b.m_arr[i];
		}
		return *this;
	}

	ANKI_ENABLE_METHOD(HAS_VEC4_SIMD)
	TVec& operator*=(const TVec& b)
	{
		m_simd = _mm_mul_ps(m_simd, b.m_simd);
		return *this;
	}

	ANKI_ENABLE_METHOD(!HAS_VEC4_SIMD)
	TVec operator/(const TVec& b) const
	{
		TVec out;
		for(U i = 0; i < N; i++)
		{
			ANKI_ASSERT(b.m_arr[i] != 0.0);
			out.m_arr[i] = m_arr[i] / b.m_arr[i];
		}
		return out;
	}

	ANKI_ENABLE_METHOD(HAS_VEC4_SIMD)
	TVec operator/(const TVec& b) const
	{
		return TVec(_mm_div_ps(m_simd, b.m_simd));
	}

	ANKI_ENABLE_METHOD(!HAS_VEC4_SIMD)
	TVec& operator/=(const TVec& b)
	{
		for(U i = 0; i < N; i++)
		{
			ANKI_ASSERT(b.m_arr[i] != 0.0);
			m_arr[i] /= b.m_arr[i];
		}
		return *this;
	}

	ANKI_ENABLE_METHOD(HAS_VEC4_SIMD)
	TVec& operator/=(const TVec& b)
	{
		m_simd = _mm_div_ps(m_simd, b.m_simd);
		return *this;
	}

	ANKI_ENABLE_METHOD(!HAS_VEC4_SIMD)
	TVec operator-() const
	{
		TVec out;
		for(U i = 0; i < N; i++)
		{
			out.m_arr[i] = -m_arr[i];
		}
		return out;
	}

	ANKI_ENABLE_METHOD(HAS_VEC4_SIMD)
	TVec operator-() const
	{
		return TVec(_mm_xor_ps(m_simd, _mm_set1_ps(-0.0)));
	}

	ANKI_ENABLE_METHOD(IS_INTEGER)
	TVec operator<<(const TVec& b) const
	{
		TVec out;
		for(U i = 0; i < N; i++)
		{
			out.m_carr[i] = m_carr[i] << b.m_carr[i];
		}
		return out;
	}

	ANKI_ENABLE_METHOD(IS_INTEGER)
	TVec& operator<<=(const TVec& b)
	{
		for(U i = 0; i < N; i++)
		{
			m_carr[i] <<= b.m_carr[i];
		}
		return *this;
	}

	ANKI_ENABLE_METHOD(IS_INTEGER)
	TVec operator>>(const TVec& b) const
	{
		TVec out;
		for(U i = 0; i < N; i++)
		{
			out.m_carr[i] = m_carr[i] >> b.m_carr[i];
		}
		return out;
	}

	ANKI_ENABLE_METHOD(IS_INTEGER)
	TVec& operator>>=(const TVec& b)
	{
		for(U i = 0; i < N; i++)
		{
			m_carr[i] >>= b.m_carr[i];
		}
		return *this;
	}

	ANKI_ENABLE_METHOD(IS_INTEGER)
	TVec operator&(const TVec& b) const
	{
		TVec out;
		for(U i = 0; i < N; i++)
		{
			out.m_carr[i] = m_carr[i] & b.m_carr[i];
		}
		return out;
	}

	ANKI_ENABLE_METHOD(IS_INTEGER)
	TVec& operator&=(const TVec& b)
	{
		for(U i = 0; i < N; i++)
		{
			m_carr[i] &= b.m_carr[i];
		}
		return *this;
	}

	ANKI_ENABLE_METHOD(IS_INTEGER)
	TVec operator|(const TVec& b) const
	{
		TVec out;
		for(U i = 0; i < N; i++)
		{
			out.m_carr[i] = m_carr[i] | b.m_carr[i];
		}
		return out;
	}

	ANKI_ENABLE_METHOD(IS_INTEGER)
	TVec& operator|=(const TVec& b)
	{
		for(U i = 0; i < N; i++)
		{
			m_carr[i] |= b.m_carr[i];
		}
		return *this;
	}

	ANKI_ENABLE_METHOD(IS_INTEGER)
	TVec operator^(const TVec& b) const
	{
		TVec out;
		for(U i = 0; i < N; i++)
		{
			out.m_carr[i] = m_carr[i] ^ b.m_carr[i];
		}
		return out;
	}

	ANKI_ENABLE_METHOD(IS_INTEGER)
	TVec& operator^=(const TVec& b)
	{
		for(U i = 0; i < N; i++)
		{
			m_carr[i] ^= b.m_carr[i];
		}
		return *this;
	}

	Bool operator==(const TVec& b) const
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

	Bool operator!=(const TVec& b) const
	{
		return !operator==(b);
	}

	Bool operator<(const TVec& b) const
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

	Bool operator<=(const TVec& b) const
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

	Bool operator>(const TVec& b) const
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

	Bool operator>=(const TVec& b) const
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
	TVec operator+(const T f) const
	{
		return (*this) + TVec(f);
	}

	TVec& operator+=(const T f)
	{
		(*this) += TVec(f);
		return *this;
	}

	TVec operator-(const T f) const
	{
		return (*this) - TVec(f);
	}

	TVec& operator-=(const T f)
	{
		(*this) -= TVec(f);
		return *this;
	}

	TVec operator*(const T f) const
	{
		return (*this) * TVec(f);
	}

	TVec& operator*=(const T f)
	{
		(*this) *= TVec(f);
		return *this;
	}

	TVec operator/(const T f) const
	{
		return (*this) / TVec(f);
	}

	TVec& operator/=(const T f)
	{
		(*this) /= TVec(f);
		return *this;
	}

	ANKI_ENABLE_METHOD(IS_INTEGER)
	TVec operator<<(const T f) const
	{
		return (*this) << TVec(f);
	}

	ANKI_ENABLE_METHOD(IS_INTEGER)
	TVec& operator<<=(const T f)
	{
		(*this) <<= TVec(f);
		return *this;
	}

	ANKI_ENABLE_METHOD(IS_INTEGER)
	TVec operator>>(const T f) const
	{
		return (*this) >> TVec(f);
	}

	ANKI_ENABLE_METHOD(IS_INTEGER)
	TVec& operator>>=(const T f)
	{
		(*this) >>= TVec(f);
		return *this;
	}

	ANKI_ENABLE_METHOD(IS_INTEGER)
	TVec operator&(const T f) const
	{
		return (*this) & TVec(f);
	}

	ANKI_ENABLE_METHOD(IS_INTEGER)
	TVec& operator&=(const T f)
	{
		(*this) &= TVec(f);
		return *this;
	}

	ANKI_ENABLE_METHOD(IS_INTEGER)
	TVec operator|(const T f) const
	{
		return (*this) | TVec(f);
	}

	ANKI_ENABLE_METHOD(IS_INTEGER)
	TVec& operator|=(const T f)
	{
		(*this) |= TVec(f);
		return *this;
	}

	ANKI_ENABLE_METHOD(IS_INTEGER)
	TVec operator^(const T f) const
	{
		return (*this) ^ TVec(f);
	}

	ANKI_ENABLE_METHOD(IS_INTEGER)
	TVec& operator^=(const T f)
	{
		(*this) ^= TVec(f);
		return *this;
	}

	Bool operator==(const T f) const
	{
		return *this == TVec(f);
	}

	Bool operator!=(const T f) const
	{
		return *this != TVec(f);
	}

	Bool operator<(const T f) const
	{
		return *this < TVec(f);
	}

	Bool operator<=(const T f) const
	{
		return *this <= TVec(f);
	}

	Bool operator>(const T f) const
	{
		return *this > TVec(f);
	}

	Bool operator>=(const T f) const
	{
		return *this >= TVec(f);
	}
	/// @}

	/// @name Operators with other
	/// @{

	/// @note 16 muls 12 adds
	ANKI_ENABLE_METHOD(N == 4)
	TVec operator*(const TMat<T, 4, 4>& m4) const
	{
		return TVec(x() * m4(0, 0) + y() * m4(1, 0) + z() * m4(2, 0) + w() * m4(3, 0),
					x() * m4(0, 1) + y() * m4(1, 1) + z() * m4(2, 1) + w() * m4(3, 1),
					x() * m4(0, 2) + y() * m4(1, 2) + z() * m4(2, 2) + w() * m4(3, 2),
					x() * m4(0, 3) + y() * m4(1, 3) + z() * m4(2, 3) + w() * m4(3, 3));
	}
	/// @}

	/// @name Other
	/// @{
	ANKI_ENABLE_METHOD(!HAS_VEC4_SIMD)
	T dot(const TVec& b) const
	{
		T out = T(0);
		for(U i = 0; i < N; i++)
		{
			out += m_arr[i] * b.m_arr[i];
		}
		return out;
	}

	ANKI_ENABLE_METHOD(HAS_VEC4_SIMD)
	T dot(const TVec& b) const
	{
		T o;
		_mm_store_ss(&o, _mm_dp_ps(m_simd, b.m_simd, 0xF1));
		return o;
	}

	/// 6 muls, 3 adds
	ANKI_ENABLE_METHOD(N == 3)
	TVec cross(const TVec& b) const
	{
		return TVec(y() * b.z() - z() * b.y(), z() * b.x() - x() * b.z(), x() * b.y() - y() * b.x());
	}

	/// It's like calculating the cross of a 3 component TVec.
	ANKI_ENABLE_METHOD(N == 4 && !HAS_VEC4_SIMD)
	TVec cross(const TVec& b) const
	{
		ANKI_ASSERT(w() == T(0));
		ANKI_ASSERT(b.w() == T(0));
		return TVec(xyz().cross(b.xyz()), T(0));
	}

	ANKI_ENABLE_METHOD(N == 4 && HAS_VEC4_SIMD)
	TVec cross(const TVec& b) const
	{
		ANKI_ASSERT(w() == T(0));
		ANKI_ASSERT(b.w() == T(0));
		const auto& a = *this;
		constexpr unsigned int mask0 = _MM_SHUFFLE(3, 0, 2, 1);
		constexpr unsigned int mask1 = _MM_SHUFFLE(3, 1, 0, 2);

		const __m128 tmp0 =
			_mm_mul_ps(_mm_shuffle_ps(a.m_simd, a.m_simd, U8(mask0)), _mm_shuffle_ps(b.m_simd, b.m_simd, U8(mask1)));
		const __m128 tmp1 =
			_mm_mul_ps(_mm_shuffle_ps(a.m_simd, a.m_simd, U8(mask1)), _mm_shuffle_ps(b.m_simd, b.m_simd, U8(mask0)));

		return TVec(_mm_sub_ps(tmp0, tmp1));
	}

	ANKI_ENABLE_METHOD(N == 3)
	TVec projectTo(const TVec& toThis) const
	{
		return toThis * ((*this).dot(toThis) / (toThis.dot(toThis)));
	}

	ANKI_ENABLE_METHOD(N == 4)
	TVec projectTo(const TVec& toThis) const
	{
		ANKI_ASSERT(w() == T(0));
		return (toThis * ((*this).dot(toThis) / (toThis.dot(toThis)))).xyz0();
	}

	ANKI_ENABLE_METHOD(N == 3)
	TVec projectTo(const TVec& rayOrigin, const TVec& rayDir) const
	{
		const auto& a = *this;
		return rayOrigin + rayDir * ((a - rayOrigin).dot(rayDir));
	}

	ANKI_ENABLE_METHOD(N == 4)
	TVec projectTo(const TVec& rayOrigin, const TVec& rayDir) const
	{
		ANKI_ASSERT(w() == T(0));
		ANKI_ASSERT(rayOrigin.w() == T(0));
		ANKI_ASSERT(rayDir.w() == T(0));
		const auto& a = *this;
		return rayOrigin + rayDir * ((a - rayOrigin).dot(rayDir));
	}

	/// Perspective divide. Divide the xyzw of this to the w of this. This method will handle some edge cases.
	ANKI_ENABLE_METHOD(N == 4)
	TVec perspectiveDivide() const
	{
		auto invw = T(1) / w(); // This may become (+-)inf
		invw = (invw > 1e+11) ? 1e+11 : invw; // Clamp
		invw = (invw < -1e+11) ? -1e+11 : invw; // Clamp
		return (*this) * invw;
	}

	ANKI_ENABLE_METHOD(!HAS_VEC4_SIMD)
	T getLengthSquared() const
	{
		T out = T(0);
		for(U i = 0; i < N; i++)
		{
			out += m_arr[i] * m_arr[i];
		}
		return out;
	}

	ANKI_ENABLE_METHOD(HAS_VEC4_SIMD)
	T getLengthSquared() const
	{
		T o;
		_mm_store_ss(&o, _mm_dp_ps(m_simd, m_simd, 0xF1));
		return o;
	}

	T getLength() const
	{
		return sqrt<T>(getLengthSquared());
	}

	T getDistanceSquared(const TVec& b) const
	{
		return ((*this) - b).getLengthSquared();
	}

	T getDistance(const TVec& b) const
	{
		return sqrt<T>(getDistance(b));
	}

	ANKI_ENABLE_METHOD(!HAS_VEC4_SIMD)
	void normalize()
	{
		(*this) /= getLength();
	}

	ANKI_ENABLE_METHOD(HAS_VEC4_SIMD)
	void normalize()
	{
		__m128 inverseNorm = _mm_rsqrt_ps(_mm_dp_ps(m_simd, m_simd, 0xFF));
		m_simd = _mm_mul_ps(m_simd, inverseNorm);
	}

	ANKI_ENABLE_METHOD(!HAS_VEC4_SIMD)
	TVec getNormalized() const
	{
		return (*this) / getLength();
	}

	ANKI_ENABLE_METHOD(HAS_VEC4_SIMD)
	TVec getNormalized() const
	{
		__m128 inverse_norm = _mm_rsqrt_ps(_mm_dp_ps(m_simd, m_simd, 0xFF));
		return TVec(_mm_mul_ps(m_simd, inverse_norm));
	}

	/// Return lerp(this, v1, t)
	TVec lerp(const TVec& v1, T t) const
	{
		return ((*this) * (1.0 - t)) + (v1 * t);
	}

	ANKI_ENABLE_METHOD(!HAS_VEC4_SIMD)
	TVec abs() const
	{
		TVec out;
		for(U i = 0; i < N; ++i)
		{
			out[i] = absolute<T>(m_arr[i]);
		}
		return out;
	}

	ANKI_ENABLE_METHOD(HAS_VEC4_SIMD)
	TVec abs() const
	{
		const __m128 signMask = _mm_set1_ps(-0.0f);
		return TVec(_mm_andnot_ps(signMask, m_simd));
	}

	/// Get clamped between two values.
	TVec clamp(const T minv, const T maxv) const
	{
		return max(TVec(minv)).min(TVec(maxv));
	}

	/// Get clamped between two vectors.
	TVec clamp(const TVec& minv, const TVec& maxv) const
	{
		return max(minv).min(maxv);
	}

	/// Get the min of all components.
	ANKI_ENABLE_METHOD(!HAS_VEC4_SIMD)
	TVec min(const TVec& b) const
	{
		TVec out;
		for(U i = 0; i < N; ++i)
		{
			out[i] = anki::min<T>(m_arr[i], b[i]);
		}
		return out;
	}

	/// Get the min of all components.
	ANKI_ENABLE_METHOD(HAS_VEC4_SIMD)
	TVec min(const TVec& b) const
	{
		return TVec(_mm_min_ps(m_simd, b.m_simd));
	}

	/// Get the min of all components.
	TVec min(const T b) const
	{
		return min(TVec(b));
	}

	/// Get the max of all components.
	ANKI_ENABLE_METHOD(!HAS_VEC4_SIMD)
	TVec max(const TVec& b) const
	{
		TVec out;
		for(U i = 0; i < N; ++i)
		{
			out[i] = anki::max<T>(m_arr[i], b[i]);
		}
		return out;
	}

	/// Get the max of all components.
	ANKI_ENABLE_METHOD(HAS_VEC4_SIMD)
	TVec max(const TVec& b) const
	{
		return TVec(_mm_max_ps(m_simd, b.m_simd));
	}

	/// Get the max of all components.
	TVec max(const T b) const
	{
		return max(TVec(b));
	}

	/// Get a safe 1 / (*this)
	TVec reciprocal() const
	{
		TVec out;
		for(U i = 0; i < N; ++i)
		{
			out[i] = T(1) / m_arr[i];
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

	static constexpr U8 getSize()
	{
		return U8(N);
	}

	ANKI_ENABLE_METHOD(std::is_floating_point<T>::value)
	void toString(StringAuto& str) const
	{
		for(U i = 0; i < N; ++i)
		{
			str.append(StringAuto(str.getAllocator()).sprintf((i < i - N) ? "%f " : "%f", m_arr[i]));
		}
	}

	static constexpr Bool CLANG_WORKAROUND = std::is_integral<T>::value && std::is_unsigned<T>::value;
	ANKI_ENABLE_METHOD(CLANG_WORKAROUND)
	void toString(StringAuto& str) const
	{
		for(U i = 0; i < N; ++i)
		{
			str.append(StringAuto(str.getAllocator()).sprintf((i < i - N) ? "%u " : "%u", m_arr[i]));
		}
	}

	static constexpr Bool CLANG_WORKAROUND2 = std::is_integral<T>::value && std::is_signed<T>::value;
	ANKI_ENABLE_METHOD(CLANG_WORKAROUND2)
	void toString(StringAuto& str) const
	{
		for(U i = 0; i < N; ++i)
		{
			str.append(StringAuto(str.getAllocator()).sprintf((i < i - N) ? "%d " : "%d", m_arr[i]));
		}
	}
	/// @}

private:
	/// @name Data
	/// @{
	union
	{
		Array<T, N> m_arr;
		T m_carr[N]; ///< To avoid bound checks on debug builds.
		Simd m_simd;
	};
	/// @}
};

/// @memberof TVec
template<typename T, U N>
TVec<T, N> operator+(const T f, const TVec<T, N>& v)
{
	return v + f;
}

/// @memberof TVec
template<typename T, U N>
TVec<T, N> operator-(const T f, const TVec<T, N>& v)
{
	return TVec<T, N>(f) - v;
}

/// @memberof TVec
template<typename T, U N>
TVec<T, N> operator*(const T f, const TVec<T, N>& v)
{
	return v * f;
}

/// @memberof TVec
template<typename T, U N>
TVec<T, N> operator/(const T f, const TVec<T, N>& v)
{
	return TVec<T, N>(f) / v;
}

// Types

/// F32 2D vector
using Vec2 = TVec<F32, 2>;
static_assert(sizeof(Vec2) == sizeof(F32) * 2, "Incorrect size");

/// Half float 2D vector
using HVec2 = TVec<F16, 2>;

/// 32bit signed integer 2D vector
using IVec2 = TVec<I32, 2>;

/// 16bit signed integer 2D vector
using I16Vec2 = TVec<I16, 2>;

/// 8bit signed integer 2D vector
using I8Vec2 = TVec<I8, 2>;

/// 32bit unsigned integer 2D vector
using UVec2 = TVec<U32, 2>;

/// 16bit unsigned integer 2D vector
using U16Vec2 = TVec<U16, 2>;

/// 8bit unsigned integer 2D vector
using U8Vec2 = TVec<U8, 2>;

/// F32 3D vector
using Vec3 = TVec<F32, 3>;
static_assert(sizeof(Vec3) == sizeof(F32) * 3, "Incorrect size");

/// Half float 3D vector
using HVec3 = TVec<F16, 3>;

/// 32bit signed integer 3D vector
using IVec3 = TVec<I32, 3>;

/// 16bit signed integer 3D vector
using I16Vec3 = TVec<I16, 3>;

/// 8bit signed integer 3D vector
using I8Vec3 = TVec<I8, 3>;

/// 32bit unsigned integer 3D vector
using UVec3 = TVec<U32, 3>;

/// 16bit unsigned integer 3D vector
using U16Vec3 = TVec<U16, 3>;

/// 8bit unsigned integer 3D vector
using U8Vec3 = TVec<U8, 3>;

/// F32 4D vector
using Vec4 = TVec<F32, 4>;
static_assert(sizeof(Vec4) == sizeof(F32) * 4, "Incorrect size");

/// Half float 4D vector
using HVec4 = TVec<F16, 4>;

/// 32bit signed integer 4D vector
using IVec4 = TVec<I32, 4>;

/// 16bit signed integer 4D vector
using I16Vec4 = TVec<I16, 4>;

/// 8bit signed integer 4D vector
using I8Vec4 = TVec<I8, 4>;

/// 32bit unsigned integer 4D vector
using UVec4 = TVec<U32, 4>;

/// 16bit unsigned integer 4D vector
using U16Vec4 = TVec<U16, 4>;

/// 8bit unsigned integer 4D vector
using U8Vec4 = TVec<U8, 4>;
/// @}

} // end namespace anki
