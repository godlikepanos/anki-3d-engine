// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Math/Common.h>
#include <AnKi/Util/F16.h>

namespace anki {

/// @addtogroup math
/// @{

/// Common code for all vectors
/// @tparam T The scalar type. Eg float.
/// @tparam kTComponentCount The number of the vector components (2, 3 or 4).
template<typename T, U kTComponentCount>
class alignas(MathSimd<T, kTComponentCount>::kAlignment) TVec
{
public:
	friend class TVec<T, 2>;
	friend class TVec<T, 3>;
	friend class TVec<T, 4>;

	using Scalar = T;
	using Simd = typename MathSimd<T, kTComponentCount>::Type;
	static constexpr U kComponentCount = kTComponentCount;
	static constexpr Bool kIsInteger = std::is_integral<T>::value;
	static constexpr Bool kVec4Simd = kTComponentCount == 4 && std::is_same<T, F32>::value && ANKI_ENABLE_SIMD;

	/// @name Constructors
	/// @{

	/// Defaut constructor. It will zero it.
	constexpr TVec()
		: TVec(T(0))
	{
	}

	/// Copy.
	TVec(const TVec& b) requires(!kVec4Simd)
	{
		for(U i = 0; i < kTComponentCount; i++)
		{
			m_arr[i] = b.m_arr[i];
		}
	}

	/// Copy.
	TVec(const TVec& b) requires(kVec4Simd)
	{
		m_simd = b.m_simd;
	}

	/// Convert from another type. From int to float vectors and the opposite.
	template<typename Y>
	explicit TVec(const TVec<Y, kTComponentCount>& b) requires(!std::is_same<Y, T>::value)
	{
		for(U i = 0; i < kTComponentCount; i++)
		{
			m_arr[i] = T(b[i]);
		}
	}

	explicit TVec(const T f) requires(!kVec4Simd)
	{
		for(U i = 0; i < kTComponentCount; ++i)
		{
			m_arr[i] = f;
		}
	}

#if ANKI_ENABLE_SIMD
	explicit TVec(const T f) requires(kVec4Simd)
	{
#	if ANKI_SIMD_SSE
		m_simd = _mm_set1_ps(f);
#	else
		m_simd = vdupq_n_f32(f);
#	endif
	}
#endif

	explicit TVec(const T arr[]) requires(!kVec4Simd)
	{
		for(U i = 0; i < kTComponentCount; ++i)
		{
			m_arr[i] = arr[i];
		}
	}

	explicit TVec(const T arr[]) requires(kVec4Simd)
	{
		m_simd = _mm_load_ps(arr);
	}

	explicit TVec(const Array<T, kTComponentCount>& arr)
	{
		for(U i = 0; i < kTComponentCount; ++i)
		{
			m_arr[i] = arr[i];
		}
	}

	explicit TVec(const Simd& simd)
	{
		m_simd = simd;
	}

	constexpr TVec(const T x_, const T y_) requires(kTComponentCount == 2)
		: m_arr{x_, y_}
	{
	}

	// Vec3 specific

	constexpr TVec(const T x_, const T y_, const T z_) requires(kTComponentCount == 3)
		: m_arr{x_, y_, z_}
	{
	}

	constexpr TVec(const TVec<T, 2>& a, const T z_) requires(kTComponentCount == 3)
		: m_arr{a.m_arr[0], a.m_arr[1], z_}
	{
	}

	constexpr TVec(const T x_, const TVec<T, 2>& a) requires(kTComponentCount == 3)
		: m_arr{x_, a.m_arr[0], a.m_arr[1]}
	{
	}

	// Vec4 specific

	constexpr TVec(const T x_, const T y_, const T z_, const T w_) requires(kTComponentCount == 4 && !kVec4Simd)
		: m_arr{x_, y_, z_, w_}
	{
	}

#if ANKI_ENABLE_SIMD
	TVec(const T x_, const T y_, const T z_, const T w_) requires(kVec4Simd)
	{
#	if ANKI_SIMD_SSE
		m_simd = _mm_set_ps(w_, z_, y_, x_);
#	else
		alignas(16) const T data[4] = {x_, y_, z_, w_};
		m_simd = vld1q_f32(data);
#	endif
	}
#endif

	constexpr TVec(const TVec<T, 3>& a, const T w_) requires(kTComponentCount == 4)
		: m_arr{a.m_arr[0], a.m_arr[1], a.m_arr[2], w_}
	{
	}

	constexpr TVec(const T x_, const TVec<T, 3>& a) requires(kTComponentCount == 4)
		: m_arr{x_, a.m_arr[0], a.m_arr[1], a.m_arr[2]}
	{
	}

	constexpr TVec(const TVec<T, 2>& a, const T z_, const T w_) requires(kTComponentCount == 4)
		: m_arr{a.m_arr[0], a.m_arr[1], z_, w_}
	{
	}

	constexpr TVec(const T x_, const TVec<T, 2>& a, const T w_) requires(kTComponentCount == 4)
		: m_arr{x_, a.m_arr[0], a.m_arr[1], w_}
	{
	}

	constexpr TVec(const T x_, const T y_, const TVec<T, 2>& a) requires(kTComponentCount == 4)
		: m_arr{x_, y_, a.m_arr[0], a.m_arr[1]}
	{
	}

	constexpr TVec(const TVec<T, 2>& a, const TVec<T, 2>& b) requires(kTComponentCount == 4)
		: m_arr{a.m_arr[0], a.m_arr[1], b.m_arr[0], b.m_arr[1]}
	{
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

	T& z() requires(kTComponentCount > 2)
	{
		return m_arr[2];
	}

	T z() const requires(kTComponentCount > 2)
	{
		return m_arr[2];
	}

	T& w() requires(kTComponentCount > 3)
	{
		return m_arr[3];
	}

	T w() const requires(kTComponentCount > 3)
	{
		return m_arr[3];
	}

	TVec<T, 4> xyz1() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(x(), y(), z(), T(1));
	}

	TVec<T, 4> xyz0() const requires(kTComponentCount > 2)
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

	TVec<T, 2> xx() const requires(kTComponentCount > 0)
	{
		return TVec<T, 2>(m_carr[0], m_carr[0]);
	}

	TVec<T, 2> xy() const requires(kTComponentCount > 1)
	{
		return TVec<T, 2>(m_carr[0], m_carr[1]);
	}

	TVec<T, 2> xz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 2>(m_carr[0], m_carr[2]);
	}

	TVec<T, 2> xw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 2>(m_carr[0], m_carr[3]);
	}

	TVec<T, 2> yx() const requires(kTComponentCount > 1)
	{
		return TVec<T, 2>(m_carr[1], m_carr[0]);
	}

	TVec<T, 2> yy() const requires(kTComponentCount > 1)
	{
		return TVec<T, 2>(m_carr[1], m_carr[1]);
	}

	TVec<T, 2> yz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 2>(m_carr[1], m_carr[2]);
	}

	TVec<T, 2> yw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 2>(m_carr[1], m_carr[3]);
	}

	TVec<T, 2> zx() const requires(kTComponentCount > 2)
	{
		return TVec<T, 2>(m_carr[2], m_carr[0]);
	}

	TVec<T, 2> zy() const requires(kTComponentCount > 2)
	{
		return TVec<T, 2>(m_carr[2], m_carr[1]);
	}

	TVec<T, 2> zz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 2>(m_carr[2], m_carr[2]);
	}

	TVec<T, 2> zw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 2>(m_carr[2], m_carr[3]);
	}

	TVec<T, 2> wx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 2>(m_carr[3], m_carr[0]);
	}

	TVec<T, 2> wy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 2>(m_carr[3], m_carr[1]);
	}

	TVec<T, 2> wz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 2>(m_carr[3], m_carr[2]);
	}

	TVec<T, 2> ww() const requires(kTComponentCount > 3)
	{
		return TVec<T, 2>(m_carr[3], m_carr[3]);
	}

	TVec<T, 3> xxx() const requires(kTComponentCount > 0)
	{
		return TVec<T, 3>(m_carr[0], m_carr[0], m_carr[0]);
	}

	TVec<T, 3> xxy() const requires(kTComponentCount > 1)
	{
		return TVec<T, 3>(m_carr[0], m_carr[0], m_carr[1]);
	}

	TVec<T, 3> xxz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 3>(m_carr[0], m_carr[0], m_carr[2]);
	}

	TVec<T, 3> xxw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[0], m_carr[0], m_carr[3]);
	}

	TVec<T, 3> xyx() const requires(kTComponentCount > 1)
	{
		return TVec<T, 3>(m_carr[0], m_carr[1], m_carr[0]);
	}

	TVec<T, 3> xyy() const requires(kTComponentCount > 1)
	{
		return TVec<T, 3>(m_carr[0], m_carr[1], m_carr[1]);
	}

	TVec<T, 3> xyz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 3>(m_carr[0], m_carr[1], m_carr[2]);
	}

	TVec<T, 3> xyw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[0], m_carr[1], m_carr[3]);
	}

	TVec<T, 3> xzx() const requires(kTComponentCount > 2)
	{
		return TVec<T, 3>(m_carr[0], m_carr[2], m_carr[0]);
	}

	TVec<T, 3> xzy() const requires(kTComponentCount > 2)
	{
		return TVec<T, 3>(m_carr[0], m_carr[2], m_carr[1]);
	}

	TVec<T, 3> xzz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 3>(m_carr[0], m_carr[2], m_carr[2]);
	}

	TVec<T, 3> xzw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[0], m_carr[2], m_carr[3]);
	}

	TVec<T, 3> xwx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[0], m_carr[3], m_carr[0]);
	}

	TVec<T, 3> xwy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[0], m_carr[3], m_carr[1]);
	}

	TVec<T, 3> xwz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[0], m_carr[3], m_carr[2]);
	}

	TVec<T, 3> xww() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[0], m_carr[3], m_carr[3]);
	}

	TVec<T, 3> yxx() const requires(kTComponentCount > 1)
	{
		return TVec<T, 3>(m_carr[1], m_carr[0], m_carr[0]);
	}

	TVec<T, 3> yxy() const requires(kTComponentCount > 1)
	{
		return TVec<T, 3>(m_carr[1], m_carr[0], m_carr[1]);
	}

	TVec<T, 3> yxz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 3>(m_carr[1], m_carr[0], m_carr[2]);
	}

	TVec<T, 3> yxw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[1], m_carr[0], m_carr[3]);
	}

	TVec<T, 3> yyx() const requires(kTComponentCount > 1)
	{
		return TVec<T, 3>(m_carr[1], m_carr[1], m_carr[0]);
	}

	TVec<T, 3> yyy() const requires(kTComponentCount > 1)
	{
		return TVec<T, 3>(m_carr[1], m_carr[1], m_carr[1]);
	}

	TVec<T, 3> yyz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 3>(m_carr[1], m_carr[1], m_carr[2]);
	}

	TVec<T, 3> yyw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[1], m_carr[1], m_carr[3]);
	}

	TVec<T, 3> yzx() const requires(kTComponentCount > 2)
	{
		return TVec<T, 3>(m_carr[1], m_carr[2], m_carr[0]);
	}

	TVec<T, 3> yzy() const requires(kTComponentCount > 2)
	{
		return TVec<T, 3>(m_carr[1], m_carr[2], m_carr[1]);
	}

	TVec<T, 3> yzz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 3>(m_carr[1], m_carr[2], m_carr[2]);
	}

	TVec<T, 3> yzw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[1], m_carr[2], m_carr[3]);
	}

	TVec<T, 3> ywx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[1], m_carr[3], m_carr[0]);
	}

	TVec<T, 3> ywy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[1], m_carr[3], m_carr[1]);
	}

	TVec<T, 3> ywz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[1], m_carr[3], m_carr[2]);
	}

	TVec<T, 3> yww() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[1], m_carr[3], m_carr[3]);
	}

	TVec<T, 3> zxx() const requires(kTComponentCount > 2)
	{
		return TVec<T, 3>(m_carr[2], m_carr[0], m_carr[0]);
	}

	TVec<T, 3> zxy() const requires(kTComponentCount > 2)
	{
		return TVec<T, 3>(m_carr[2], m_carr[0], m_carr[1]);
	}

	TVec<T, 3> zxz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 3>(m_carr[2], m_carr[0], m_carr[2]);
	}

	TVec<T, 3> zxw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[2], m_carr[0], m_carr[3]);
	}

	TVec<T, 3> zyx() const requires(kTComponentCount > 2)
	{
		return TVec<T, 3>(m_carr[2], m_carr[1], m_carr[0]);
	}

	TVec<T, 3> zyy() const requires(kTComponentCount > 2)
	{
		return TVec<T, 3>(m_carr[2], m_carr[1], m_carr[1]);
	}

	TVec<T, 3> zyz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 3>(m_carr[2], m_carr[1], m_carr[2]);
	}

	TVec<T, 3> zyw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[2], m_carr[1], m_carr[3]);
	}

	TVec<T, 3> zzx() const requires(kTComponentCount > 2)
	{
		return TVec<T, 3>(m_carr[2], m_carr[2], m_carr[0]);
	}

	TVec<T, 3> zzy() const requires(kTComponentCount > 2)
	{
		return TVec<T, 3>(m_carr[2], m_carr[2], m_carr[1]);
	}

	TVec<T, 3> zzz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 3>(m_carr[2], m_carr[2], m_carr[2]);
	}

	TVec<T, 3> zzw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[2], m_carr[2], m_carr[3]);
	}

	TVec<T, 3> zwx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[2], m_carr[3], m_carr[0]);
	}

	TVec<T, 3> zwy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[2], m_carr[3], m_carr[1]);
	}

	TVec<T, 3> zwz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[2], m_carr[3], m_carr[2]);
	}

	TVec<T, 3> zww() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[2], m_carr[3], m_carr[3]);
	}

	TVec<T, 3> wxx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[3], m_carr[0], m_carr[0]);
	}

	TVec<T, 3> wxy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[3], m_carr[0], m_carr[1]);
	}

	TVec<T, 3> wxz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[3], m_carr[0], m_carr[2]);
	}

	TVec<T, 3> wxw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[3], m_carr[0], m_carr[3]);
	}

	TVec<T, 3> wyx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[3], m_carr[1], m_carr[0]);
	}

	TVec<T, 3> wyy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[3], m_carr[1], m_carr[1]);
	}

	TVec<T, 3> wyz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[3], m_carr[1], m_carr[2]);
	}

	TVec<T, 3> wyw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[3], m_carr[1], m_carr[3]);
	}

	TVec<T, 3> wzx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[3], m_carr[2], m_carr[0]);
	}

	TVec<T, 3> wzy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[3], m_carr[2], m_carr[1]);
	}

	TVec<T, 3> wzz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[3], m_carr[2], m_carr[2]);
	}

	TVec<T, 3> wzw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[3], m_carr[2], m_carr[3]);
	}

	TVec<T, 3> wwx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[3], m_carr[3], m_carr[0]);
	}

	TVec<T, 3> wwy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[3], m_carr[3], m_carr[1]);
	}

	TVec<T, 3> wwz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[3], m_carr[3], m_carr[2]);
	}

	TVec<T, 3> www() const requires(kTComponentCount > 3)
	{
		return TVec<T, 3>(m_carr[3], m_carr[3], m_carr[3]);
	}

	TVec<T, 4> xxxx() const requires(kTComponentCount > 0)
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[0], m_carr[0]);
	}

	TVec<T, 4> xxxy() const requires(kTComponentCount > 1)
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[0], m_carr[1]);
	}

	TVec<T, 4> xxxz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[0], m_carr[2]);
	}

	TVec<T, 4> xxxw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[0], m_carr[3]);
	}

	TVec<T, 4> xxyx() const requires(kTComponentCount > 1)
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[1], m_carr[0]);
	}

	TVec<T, 4> xxyy() const requires(kTComponentCount > 1)
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[1], m_carr[1]);
	}

	TVec<T, 4> xxyz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[1], m_carr[2]);
	}

	TVec<T, 4> xxyw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[1], m_carr[3]);
	}

	TVec<T, 4> xxzx() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[2], m_carr[0]);
	}

	TVec<T, 4> xxzy() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[2], m_carr[1]);
	}

	TVec<T, 4> xxzz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[2], m_carr[2]);
	}

	TVec<T, 4> xxzw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[2], m_carr[3]);
	}

	TVec<T, 4> xxwx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[3], m_carr[0]);
	}

	TVec<T, 4> xxwy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[3], m_carr[1]);
	}

	TVec<T, 4> xxwz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[3], m_carr[2]);
	}

	TVec<T, 4> xxww() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[0], m_carr[3], m_carr[3]);
	}

	TVec<T, 4> xyxx() const requires(kTComponentCount > 1)
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[0], m_carr[0]);
	}

	TVec<T, 4> xyxy() const requires(kTComponentCount > 1)
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[0], m_carr[1]);
	}

	TVec<T, 4> xyxz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[0], m_carr[2]);
	}

	TVec<T, 4> xyxw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[0], m_carr[3]);
	}

	TVec<T, 4> xyyx() const requires(kTComponentCount > 1)
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[1], m_carr[0]);
	}

	TVec<T, 4> xyyy() const requires(kTComponentCount > 1)
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[1], m_carr[1]);
	}

	TVec<T, 4> xyyz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[1], m_carr[2]);
	}

	TVec<T, 4> xyyw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[1], m_carr[3]);
	}

	TVec<T, 4> xyzx() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[2], m_carr[0]);
	}

	TVec<T, 4> xyzy() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[2], m_carr[1]);
	}

	TVec<T, 4> xyzz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[2], m_carr[2]);
	}

	TVec<T, 4> xyzw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[2], m_carr[3]);
	}

	TVec<T, 4> xywx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[3], m_carr[0]);
	}

	TVec<T, 4> xywy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[3], m_carr[1]);
	}

	TVec<T, 4> xywz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[3], m_carr[2]);
	}

	TVec<T, 4> xyww() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[1], m_carr[3], m_carr[3]);
	}

	TVec<T, 4> xzxx() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[0], m_carr[0]);
	}

	TVec<T, 4> xzxy() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[0], m_carr[1]);
	}

	TVec<T, 4> xzxz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[0], m_carr[2]);
	}

	TVec<T, 4> xzxw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[0], m_carr[3]);
	}

	TVec<T, 4> xzyx() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[1], m_carr[0]);
	}

	TVec<T, 4> xzyy() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[1], m_carr[1]);
	}

	TVec<T, 4> xzyz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[1], m_carr[2]);
	}

	TVec<T, 4> xzyw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[1], m_carr[3]);
	}

	TVec<T, 4> xzzx() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[2], m_carr[0]);
	}

	TVec<T, 4> xzzy() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[2], m_carr[1]);
	}

	TVec<T, 4> xzzz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[2], m_carr[2]);
	}

	TVec<T, 4> xzzw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[2], m_carr[3]);
	}

	TVec<T, 4> xzwx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[3], m_carr[0]);
	}

	TVec<T, 4> xzwy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[3], m_carr[1]);
	}

	TVec<T, 4> xzwz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[3], m_carr[2]);
	}

	TVec<T, 4> xzww() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[2], m_carr[3], m_carr[3]);
	}

	TVec<T, 4> xwxx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[0], m_carr[0]);
	}

	TVec<T, 4> xwxy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[0], m_carr[1]);
	}

	TVec<T, 4> xwxz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[0], m_carr[2]);
	}

	TVec<T, 4> xwxw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[0], m_carr[3]);
	}

	TVec<T, 4> xwyx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[1], m_carr[0]);
	}

	TVec<T, 4> xwyy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[1], m_carr[1]);
	}

	TVec<T, 4> xwyz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[1], m_carr[2]);
	}

	TVec<T, 4> xwyw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[1], m_carr[3]);
	}

	TVec<T, 4> xwzx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[2], m_carr[0]);
	}

	TVec<T, 4> xwzy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[2], m_carr[1]);
	}

	TVec<T, 4> xwzz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[2], m_carr[2]);
	}

	TVec<T, 4> xwzw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[2], m_carr[3]);
	}

	TVec<T, 4> xwwx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[3], m_carr[0]);
	}

	TVec<T, 4> xwwy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[3], m_carr[1]);
	}

	TVec<T, 4> xwwz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[3], m_carr[2]);
	}

	TVec<T, 4> xwww() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[0], m_carr[3], m_carr[3], m_carr[3]);
	}

	TVec<T, 4> yxxx() const requires(kTComponentCount > 1)
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[0], m_carr[0]);
	}

	TVec<T, 4> yxxy() const requires(kTComponentCount > 1)
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[0], m_carr[1]);
	}

	TVec<T, 4> yxxz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[0], m_carr[2]);
	}

	TVec<T, 4> yxxw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[0], m_carr[3]);
	}

	TVec<T, 4> yxyx() const requires(kTComponentCount > 1)
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[1], m_carr[0]);
	}

	TVec<T, 4> yxyy() const requires(kTComponentCount > 1)
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[1], m_carr[1]);
	}

	TVec<T, 4> yxyz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[1], m_carr[2]);
	}

	TVec<T, 4> yxyw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[1], m_carr[3]);
	}

	TVec<T, 4> yxzx() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[2], m_carr[0]);
	}

	TVec<T, 4> yxzy() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[2], m_carr[1]);
	}

	TVec<T, 4> yxzz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[2], m_carr[2]);
	}

	TVec<T, 4> yxzw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[2], m_carr[3]);
	}

	TVec<T, 4> yxwx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[3], m_carr[0]);
	}

	TVec<T, 4> yxwy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[3], m_carr[1]);
	}

	TVec<T, 4> yxwz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[3], m_carr[2]);
	}

	TVec<T, 4> yxww() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[0], m_carr[3], m_carr[3]);
	}

	TVec<T, 4> yyxx() const requires(kTComponentCount > 1)
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[0], m_carr[0]);
	}

	TVec<T, 4> yyxy() const requires(kTComponentCount > 1)
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[0], m_carr[1]);
	}

	TVec<T, 4> yyxz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[0], m_carr[2]);
	}

	TVec<T, 4> yyxw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[0], m_carr[3]);
	}

	TVec<T, 4> yyyx() const requires(kTComponentCount > 1)
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[1], m_carr[0]);
	}

	TVec<T, 4> yyyy() const requires(kTComponentCount > 1)
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[1], m_carr[1]);
	}

	TVec<T, 4> yyyz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[1], m_carr[2]);
	}

	TVec<T, 4> yyyw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[1], m_carr[3]);
	}

	TVec<T, 4> yyzx() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[2], m_carr[0]);
	}

	TVec<T, 4> yyzy() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[2], m_carr[1]);
	}

	TVec<T, 4> yyzz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[2], m_carr[2]);
	}

	TVec<T, 4> yyzw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[2], m_carr[3]);
	}

	TVec<T, 4> yywx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[3], m_carr[0]);
	}

	TVec<T, 4> yywy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[3], m_carr[1]);
	}

	TVec<T, 4> yywz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[3], m_carr[2]);
	}

	TVec<T, 4> yyww() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[1], m_carr[3], m_carr[3]);
	}

	TVec<T, 4> yzxx() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[0], m_carr[0]);
	}

	TVec<T, 4> yzxy() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[0], m_carr[1]);
	}

	TVec<T, 4> yzxz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[0], m_carr[2]);
	}

	TVec<T, 4> yzxw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[0], m_carr[3]);
	}

	TVec<T, 4> yzyx() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[1], m_carr[0]);
	}

	TVec<T, 4> yzyy() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[1], m_carr[1]);
	}

	TVec<T, 4> yzyz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[1], m_carr[2]);
	}

	TVec<T, 4> yzyw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[1], m_carr[3]);
	}

	TVec<T, 4> yzzx() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[2], m_carr[0]);
	}

	TVec<T, 4> yzzy() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[2], m_carr[1]);
	}

	TVec<T, 4> yzzz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[2], m_carr[2]);
	}

	TVec<T, 4> yzzw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[2], m_carr[3]);
	}

	TVec<T, 4> yzwx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[3], m_carr[0]);
	}

	TVec<T, 4> yzwy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[3], m_carr[1]);
	}

	TVec<T, 4> yzwz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[3], m_carr[2]);
	}

	TVec<T, 4> yzww() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[2], m_carr[3], m_carr[3]);
	}

	TVec<T, 4> ywxx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[0], m_carr[0]);
	}

	TVec<T, 4> ywxy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[0], m_carr[1]);
	}

	TVec<T, 4> ywxz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[0], m_carr[2]);
	}

	TVec<T, 4> ywxw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[0], m_carr[3]);
	}

	TVec<T, 4> ywyx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[1], m_carr[0]);
	}

	TVec<T, 4> ywyy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[1], m_carr[1]);
	}

	TVec<T, 4> ywyz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[1], m_carr[2]);
	}

	TVec<T, 4> ywyw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[1], m_carr[3]);
	}

	TVec<T, 4> ywzx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[2], m_carr[0]);
	}

	TVec<T, 4> ywzy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[2], m_carr[1]);
	}

	TVec<T, 4> ywzz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[2], m_carr[2]);
	}

	TVec<T, 4> ywzw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[2], m_carr[3]);
	}

	TVec<T, 4> ywwx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[3], m_carr[0]);
	}

	TVec<T, 4> ywwy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[3], m_carr[1]);
	}

	TVec<T, 4> ywwz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[3], m_carr[2]);
	}

	TVec<T, 4> ywww() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[1], m_carr[3], m_carr[3], m_carr[3]);
	}

	TVec<T, 4> zxxx() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[0], m_carr[0]);
	}

	TVec<T, 4> zxxy() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[0], m_carr[1]);
	}

	TVec<T, 4> zxxz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[0], m_carr[2]);
	}

	TVec<T, 4> zxxw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[0], m_carr[3]);
	}

	TVec<T, 4> zxyx() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[1], m_carr[0]);
	}

	TVec<T, 4> zxyy() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[1], m_carr[1]);
	}

	TVec<T, 4> zxyz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[1], m_carr[2]);
	}

	TVec<T, 4> zxyw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[1], m_carr[3]);
	}

	TVec<T, 4> zxzx() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[2], m_carr[0]);
	}

	TVec<T, 4> zxzy() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[2], m_carr[1]);
	}

	TVec<T, 4> zxzz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[2], m_carr[2]);
	}

	TVec<T, 4> zxzw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[2], m_carr[3]);
	}

	TVec<T, 4> zxwx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[3], m_carr[0]);
	}

	TVec<T, 4> zxwy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[3], m_carr[1]);
	}

	TVec<T, 4> zxwz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[3], m_carr[2]);
	}

	TVec<T, 4> zxww() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[0], m_carr[3], m_carr[3]);
	}

	TVec<T, 4> zyxx() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[0], m_carr[0]);
	}

	TVec<T, 4> zyxy() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[0], m_carr[1]);
	}

	TVec<T, 4> zyxz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[0], m_carr[2]);
	}

	TVec<T, 4> zyxw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[0], m_carr[3]);
	}

	TVec<T, 4> zyyx() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[1], m_carr[0]);
	}

	TVec<T, 4> zyyy() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[1], m_carr[1]);
	}

	TVec<T, 4> zyyz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[1], m_carr[2]);
	}

	TVec<T, 4> zyyw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[1], m_carr[3]);
	}

	TVec<T, 4> zyzx() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[2], m_carr[0]);
	}

	TVec<T, 4> zyzy() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[2], m_carr[1]);
	}

	TVec<T, 4> zyzz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[2], m_carr[2]);
	}

	TVec<T, 4> zyzw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[2], m_carr[3]);
	}

	TVec<T, 4> zywx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[3], m_carr[0]);
	}

	TVec<T, 4> zywy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[3], m_carr[1]);
	}

	TVec<T, 4> zywz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[3], m_carr[2]);
	}

	TVec<T, 4> zyww() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[1], m_carr[3], m_carr[3]);
	}

	TVec<T, 4> zzxx() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[0], m_carr[0]);
	}

	TVec<T, 4> zzxy() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[0], m_carr[1]);
	}

	TVec<T, 4> zzxz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[0], m_carr[2]);
	}

	TVec<T, 4> zzxw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[0], m_carr[3]);
	}

	TVec<T, 4> zzyx() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[1], m_carr[0]);
	}

	TVec<T, 4> zzyy() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[1], m_carr[1]);
	}

	TVec<T, 4> zzyz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[1], m_carr[2]);
	}

	TVec<T, 4> zzyw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[1], m_carr[3]);
	}

	TVec<T, 4> zzzx() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[2], m_carr[0]);
	}

	TVec<T, 4> zzzy() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[2], m_carr[1]);
	}

	TVec<T, 4> zzzz() const requires(kTComponentCount > 2)
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[2], m_carr[2]);
	}

	TVec<T, 4> zzzw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[2], m_carr[3]);
	}

	TVec<T, 4> zzwx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[3], m_carr[0]);
	}

	TVec<T, 4> zzwy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[3], m_carr[1]);
	}

	TVec<T, 4> zzwz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[3], m_carr[2]);
	}

	TVec<T, 4> zzww() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[2], m_carr[3], m_carr[3]);
	}

	TVec<T, 4> zwxx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[0], m_carr[0]);
	}

	TVec<T, 4> zwxy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[0], m_carr[1]);
	}

	TVec<T, 4> zwxz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[0], m_carr[2]);
	}

	TVec<T, 4> zwxw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[0], m_carr[3]);
	}

	TVec<T, 4> zwyx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[1], m_carr[0]);
	}

	TVec<T, 4> zwyy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[1], m_carr[1]);
	}

	TVec<T, 4> zwyz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[1], m_carr[2]);
	}

	TVec<T, 4> zwyw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[1], m_carr[3]);
	}

	TVec<T, 4> zwzx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[2], m_carr[0]);
	}

	TVec<T, 4> zwzy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[2], m_carr[1]);
	}

	TVec<T, 4> zwzz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[2], m_carr[2]);
	}

	TVec<T, 4> zwzw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[2], m_carr[3]);
	}

	TVec<T, 4> zwwx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[3], m_carr[0]);
	}

	TVec<T, 4> zwwy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[3], m_carr[1]);
	}

	TVec<T, 4> zwwz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[3], m_carr[2]);
	}

	TVec<T, 4> zwww() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[2], m_carr[3], m_carr[3], m_carr[3]);
	}

	TVec<T, 4> wxxx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[0], m_carr[0]);
	}

	TVec<T, 4> wxxy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[0], m_carr[1]);
	}

	TVec<T, 4> wxxz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[0], m_carr[2]);
	}

	TVec<T, 4> wxxw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[0], m_carr[3]);
	}

	TVec<T, 4> wxyx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[1], m_carr[0]);
	}

	TVec<T, 4> wxyy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[1], m_carr[1]);
	}

	TVec<T, 4> wxyz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[1], m_carr[2]);
	}

	TVec<T, 4> wxyw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[1], m_carr[3]);
	}

	TVec<T, 4> wxzx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[2], m_carr[0]);
	}

	TVec<T, 4> wxzy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[2], m_carr[1]);
	}

	TVec<T, 4> wxzz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[2], m_carr[2]);
	}

	TVec<T, 4> wxzw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[2], m_carr[3]);
	}

	TVec<T, 4> wxwx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[3], m_carr[0]);
	}

	TVec<T, 4> wxwy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[3], m_carr[1]);
	}

	TVec<T, 4> wxwz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[3], m_carr[2]);
	}

	TVec<T, 4> wxww() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[0], m_carr[3], m_carr[3]);
	}

	TVec<T, 4> wyxx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[0], m_carr[0]);
	}

	TVec<T, 4> wyxy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[0], m_carr[1]);
	}

	TVec<T, 4> wyxz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[0], m_carr[2]);
	}

	TVec<T, 4> wyxw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[0], m_carr[3]);
	}

	TVec<T, 4> wyyx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[1], m_carr[0]);
	}

	TVec<T, 4> wyyy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[1], m_carr[1]);
	}

	TVec<T, 4> wyyz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[1], m_carr[2]);
	}

	TVec<T, 4> wyyw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[1], m_carr[3]);
	}

	TVec<T, 4> wyzx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[2], m_carr[0]);
	}

	TVec<T, 4> wyzy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[2], m_carr[1]);
	}

	TVec<T, 4> wyzz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[2], m_carr[2]);
	}

	TVec<T, 4> wyzw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[2], m_carr[3]);
	}

	TVec<T, 4> wywx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[3], m_carr[0]);
	}

	TVec<T, 4> wywy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[3], m_carr[1]);
	}

	TVec<T, 4> wywz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[3], m_carr[2]);
	}

	TVec<T, 4> wyww() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[1], m_carr[3], m_carr[3]);
	}

	TVec<T, 4> wzxx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[0], m_carr[0]);
	}

	TVec<T, 4> wzxy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[0], m_carr[1]);
	}

	TVec<T, 4> wzxz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[0], m_carr[2]);
	}

	TVec<T, 4> wzxw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[0], m_carr[3]);
	}

	TVec<T, 4> wzyx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[1], m_carr[0]);
	}

	TVec<T, 4> wzyy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[1], m_carr[1]);
	}

	TVec<T, 4> wzyz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[1], m_carr[2]);
	}

	TVec<T, 4> wzyw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[1], m_carr[3]);
	}

	TVec<T, 4> wzzx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[2], m_carr[0]);
	}

	TVec<T, 4> wzzy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[2], m_carr[1]);
	}

	TVec<T, 4> wzzz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[2], m_carr[2]);
	}

	TVec<T, 4> wzzw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[2], m_carr[3]);
	}

	TVec<T, 4> wzwx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[3], m_carr[0]);
	}

	TVec<T, 4> wzwy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[3], m_carr[1]);
	}

	TVec<T, 4> wzwz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[3], m_carr[2]);
	}

	TVec<T, 4> wzww() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[2], m_carr[3], m_carr[3]);
	}

	TVec<T, 4> wwxx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[0], m_carr[0]);
	}

	TVec<T, 4> wwxy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[0], m_carr[1]);
	}

	TVec<T, 4> wwxz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[0], m_carr[2]);
	}

	TVec<T, 4> wwxw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[0], m_carr[3]);
	}

	TVec<T, 4> wwyx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[1], m_carr[0]);
	}

	TVec<T, 4> wwyy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[1], m_carr[1]);
	}

	TVec<T, 4> wwyz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[1], m_carr[2]);
	}

	TVec<T, 4> wwyw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[1], m_carr[3]);
	}

	TVec<T, 4> wwzx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[2], m_carr[0]);
	}

	TVec<T, 4> wwzy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[2], m_carr[1]);
	}

	TVec<T, 4> wwzz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[2], m_carr[2]);
	}

	TVec<T, 4> wwzw() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[2], m_carr[3]);
	}

	TVec<T, 4> wwwx() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[3], m_carr[0]);
	}

	TVec<T, 4> wwwy() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[3], m_carr[1]);
	}

	TVec<T, 4> wwwz() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[3], m_carr[2]);
	}

	TVec<T, 4> wwww() const requires(kTComponentCount > 3)
	{
		return TVec<T, 4>(m_carr[3], m_carr[3], m_carr[3], m_carr[3]);
	}
	/// @}

	/// @name Operators with same type
	/// @{

	// Copy
	TVec& operator=(const TVec& b) requires(!kVec4Simd)
	{
		for(U i = 0; i < kTComponentCount; i++)
		{
			m_arr[i] = b.m_carr[i];
		}
		return *this;
	}

	// Copy
	TVec& operator=(const TVec& b) requires(kVec4Simd)
	{
		m_simd = b.m_simd;
		return *this;
	}

	[[nodiscard]] TVec operator+(const TVec& b) const requires(!kVec4Simd)
	{
		TVec out;
		for(U i = 0; i < kTComponentCount; i++)
		{
			out.m_arr[i] = m_arr[i] + b.m_arr[i];
		}
		return out;
	}

#if ANKI_ENABLE_SIMD
	[[nodiscard]] TVec operator+(const TVec& b) const requires(kVec4Simd)
	{
#	if ANKI_SIMD_SSE
		return TVec(_mm_add_ps(m_simd, b.m_simd));
#	else
		return TVec(vaddq_f32(m_simd, b.m_simd));
#	endif
	}
#endif

	TVec& operator+=(const TVec& b) requires(!kVec4Simd)
	{
		for(U i = 0; i < kTComponentCount; i++)
		{
			m_arr[i] += b.m_arr[i];
		}
		return *this;
	}

#if ANKI_ENABLE_SIMD
	TVec& operator+=(const TVec& b) requires(kVec4Simd)
	{
#	if ANKI_SIMD_SSE
		m_simd = _mm_add_ps(m_simd, b.m_simd);
#	else
		m_simd = vaddq_f32(m_simd, b.m_simd);
#	endif
		return *this;
	}
#endif

	[[nodiscard]] TVec operator-(const TVec& b) const requires(!kVec4Simd)
	{
		TVec out;
		for(U i = 0; i < kTComponentCount; i++)
		{
			out.m_arr[i] = m_arr[i] - b.m_arr[i];
		}
		return out;
	}

#if ANKI_ENABLE_SIMD
	[[nodiscard]] TVec operator-(const TVec& b) const requires(kVec4Simd)
	{
#	if ANKI_SIMD_SSE
		return TVec(_mm_sub_ps(m_simd, b.m_simd));
#	else
		return TVec(vsubq_f32(m_simd, b.m_simd));
#	endif
	}
#endif

	TVec& operator-=(const TVec& b) requires(!kVec4Simd)
	{
		for(U i = 0; i < kTComponentCount; i++)
		{
			m_arr[i] -= b.m_arr[i];
		}
		return *this;
	}

#if ANKI_ENABLE_SIMD
	TVec& operator-=(const TVec& b) requires(kVec4Simd)
	{
#	if ANKI_SIMD_SSE
		m_simd = _mm_sub_ps(m_simd, b.m_simd);
#	else
		m_simd = vsubq_f32(m_simd, b.m_simd);
#	endif
		return *this;
	}
#endif

	[[nodiscard]] TVec operator*(const TVec& b) const requires(!kVec4Simd)
	{
		TVec out;
		for(U i = 0; i < kTComponentCount; i++)
		{
			out.m_arr[i] = m_arr[i] * b.m_arr[i];
		}
		return out;
	}

#if ANKI_ENABLE_SIMD
	[[nodiscard]] TVec operator*(const TVec& b) const requires(kVec4Simd)
	{
#	if ANKI_SIMD_SSE
		return TVec(_mm_mul_ps(m_simd, b.m_simd));
#	else
		return TVec(vmulq_f32(m_simd, b.m_simd));
#	endif
	}
#endif

	TVec& operator*=(const TVec& b) requires(!kVec4Simd)
	{
		for(U i = 0; i < kTComponentCount; i++)
		{
			m_arr[i] *= b.m_arr[i];
		}
		return *this;
	}

#if ANKI_ENABLE_SIMD
	TVec& operator*=(const TVec& b) requires(kVec4Simd)
	{
#	if ANKI_SIMD_SSE
		m_simd = _mm_mul_ps(m_simd, b.m_simd);
#	else
		m_simd = vmulq_f32(m_simd, b.m_simd);
#	endif
		return *this;
	}
#endif

	[[nodiscard]] TVec operator/(const TVec& b) const requires(!kVec4Simd)
	{
		TVec out;
		for(U i = 0; i < kTComponentCount; i++)
		{
			ANKI_ASSERT(b.m_arr[i] != 0.0);
			out.m_arr[i] = m_arr[i] / b.m_arr[i];
		}
		return out;
	}

#if ANKI_ENABLE_SIMD
	[[nodiscard]] TVec operator/(const TVec& b) const requires(kVec4Simd)
	{
#	if ANKI_SIMD_SSE
		return TVec(_mm_div_ps(m_simd, b.m_simd));
#	else
		return TVec(vdivq_f32(m_simd, b.m_simd));
#	endif
	}
#endif

	TVec& operator/=(const TVec& b) requires(!kVec4Simd)
	{
		for(U i = 0; i < kTComponentCount; i++)
		{
			ANKI_ASSERT(b.m_arr[i] != 0.0);
			m_arr[i] /= b.m_arr[i];
		}
		return *this;
	}

#if ANKI_ENABLE_SIMD
	TVec& operator/=(const TVec& b) requires(kVec4Simd)
	{
#	if ANKI_SIMD_SSE
		m_simd = _mm_div_ps(m_simd, b.m_simd);
#	else
		m_simd = vdivq_f32(m_simd, b.m_simd);
#	endif
		return *this;
	}
#endif

	[[nodiscard]] TVec operator-() const requires(!kVec4Simd)
	{
		TVec out;
		for(U i = 0; i < kTComponentCount; i++)
		{
			out.m_arr[i] = -m_arr[i];
		}
		return out;
	}

#if ANKI_ENABLE_SIMD
	[[nodiscard]] TVec operator-() const requires(kVec4Simd)
	{
#	if ANKI_SIMD_SSE
		return TVec(_mm_xor_ps(m_simd, _mm_set1_ps(-0.0)));
#	else
		return TVec(veorq_s32(m_simd, vdupq_n_f32(-0.0)));
#	endif
	}
#endif

	[[nodiscard]] TVec operator<<(const TVec& b) const requires(kIsInteger)
	{
		TVec out;
		for(U i = 0; i < kTComponentCount; i++)
		{
			out.m_carr[i] = m_carr[i] << b.m_carr[i];
		}
		return out;
	}

	TVec& operator<<=(const TVec& b) requires(kIsInteger)
	{
		for(U i = 0; i < kTComponentCount; i++)
		{
			m_carr[i] <<= b.m_carr[i];
		}
		return *this;
	}

	[[nodiscard]] TVec operator>>(const TVec& b) const requires(kIsInteger)
	{
		TVec out;
		for(U i = 0; i < kTComponentCount; i++)
		{
			out.m_carr[i] = m_carr[i] >> b.m_carr[i];
		}
		return out;
	}

	TVec& operator>>=(const TVec& b) requires(kIsInteger)
	{
		for(U i = 0; i < kTComponentCount; i++)
		{
			m_carr[i] >>= b.m_carr[i];
		}
		return *this;
	}

	[[nodiscard]] TVec operator&(const TVec& b) const requires(kIsInteger)
	{
		TVec out;
		for(U i = 0; i < kTComponentCount; i++)
		{
			out.m_carr[i] = m_carr[i] & b.m_carr[i];
		}
		return out;
	}

	TVec& operator&=(const TVec& b) requires(kIsInteger)
	{
		for(U i = 0; i < kTComponentCount; i++)
		{
			m_carr[i] &= b.m_carr[i];
		}
		return *this;
	}

	[[nodiscard]] TVec operator|(const TVec& b) const requires(kIsInteger)
	{
		TVec out;
		for(U i = 0; i < kTComponentCount; i++)
		{
			out.m_carr[i] = m_carr[i] | b.m_carr[i];
		}
		return out;
	}

	TVec& operator|=(const TVec& b) requires(kIsInteger)
	{
		for(U i = 0; i < kTComponentCount; i++)
		{
			m_carr[i] |= b.m_carr[i];
		}
		return *this;
	}

	[[nodiscard]] TVec operator^(const TVec& b) const requires(kIsInteger)
	{
		TVec out;
		for(U i = 0; i < kTComponentCount; i++)
		{
			out.m_carr[i] = m_carr[i] ^ b.m_carr[i];
		}
		return out;
	}

	TVec& operator^=(const TVec& b) requires(kIsInteger)
	{
		for(U i = 0; i < kTComponentCount; i++)
		{
			m_carr[i] ^= b.m_carr[i];
		}
		return *this;
	}

	[[nodiscard]] TVec operator%(const TVec& b) const requires(kIsInteger)
	{
		TVec out;
		for(U i = 0; i < kTComponentCount; i++)
		{
			out.m_carr[i] = m_carr[i] % b.m_carr[i];
		}
		return out;
	}

	TVec& operator%=(const TVec& b) requires(kIsInteger)
	{
		for(U i = 0; i < kTComponentCount; i++)
		{
			m_carr[i] %= b.m_carr[i];
		}
		return *this;
	}

	[[nodiscard]] Bool operator==(const TVec& b) const
	{
		for(U i = 0; i < kTComponentCount; i++)
		{
			if(!isZero<T>(m_arr[i] - b.m_arr[i]))
			{
				return false;
			}
		}
		return true;
	}

	[[nodiscard]] Bool operator!=(const TVec& b) const
	{
		return !operator==(b);
	}

	[[nodiscard]] Bool operator<(const TVec& b) const
	{
		for(U i = 0; i < kTComponentCount; i++)
		{
			if(m_arr[i] >= b.m_arr[i])
			{
				return false;
			}
		}
		return true;
	}

	[[nodiscard]] Bool operator<=(const TVec& b) const
	{
		for(U i = 0; i < kTComponentCount; i++)
		{
			if(m_arr[i] > b.m_arr[i])
			{
				return false;
			}
		}
		return true;
	}

	[[nodiscard]] Bool operator>(const TVec& b) const
	{
		for(U i = 0; i < kTComponentCount; i++)
		{
			if(m_arr[i] <= b.m_arr[i])
			{
				return false;
			}
		}
		return true;
	}

	[[nodiscard]] Bool operator>=(const TVec& b) const
	{
		for(U i = 0; i < kTComponentCount; i++)
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
	[[nodiscard]] TVec operator+(const T f) const
	{
		return (*this) + TVec(f);
	}

	TVec& operator+=(const T f)
	{
		(*this) += TVec(f);
		return *this;
	}

	[[nodiscard]] TVec operator-(const T f) const
	{
		return (*this) - TVec(f);
	}

	TVec& operator-=(const T f)
	{
		(*this) -= TVec(f);
		return *this;
	}

	[[nodiscard]] TVec operator*(const T f) const
	{
		return (*this) * TVec(f);
	}

	TVec& operator*=(const T f)
	{
		(*this) *= TVec(f);
		return *this;
	}

	[[nodiscard]] TVec operator/(const T f) const
	{
		return (*this) / TVec(f);
	}

	TVec& operator/=(const T f)
	{
		(*this) /= TVec(f);
		return *this;
	}

	[[nodiscard]] TVec operator<<(const T f) const requires(kIsInteger)
	{
		return (*this) << TVec(f);
	}

	TVec& operator<<=(const T f) requires(kIsInteger)
	{
		(*this) <<= TVec(f);
		return *this;
	}

	[[nodiscard]] TVec operator>>(const T f) const requires(kIsInteger)
	{
		return (*this) >> TVec(f);
	}

	TVec& operator>>=(const T f) requires(kIsInteger)
	{
		(*this) >>= TVec(f);
		return *this;
	}

	[[nodiscard]] TVec operator&(const T f) const requires(kIsInteger)
	{
		return (*this) & TVec(f);
	}

	TVec& operator&=(const T f) requires(kIsInteger)
	{
		(*this) &= TVec(f);
		return *this;
	}

	[[nodiscard]] TVec operator|(const T f) const requires(kIsInteger)
	{
		return (*this) | TVec(f);
	}

	TVec& operator|=(const T f) requires(kIsInteger)
	{
		(*this) |= TVec(f);
		return *this;
	}

	[[nodiscard]] TVec operator^(const T f) const requires(kIsInteger)
	{
		return (*this) ^ TVec(f);
	}

	TVec& operator^=(const T f) requires(kIsInteger)
	{
		(*this) ^= TVec(f);
		return *this;
	}

	[[nodiscard]] TVec operator%(const T f) const requires(kIsInteger)
	{
		return (*this) % TVec(f);
	}

	TVec& operator%=(const T f) requires(kIsInteger)
	{
		(*this) %= TVec(f);
		return *this;
	}

	[[nodiscard]] Bool operator==(const T f) const
	{
		return *this == TVec(f);
	}

	[[nodiscard]] Bool operator!=(const T f) const
	{
		return *this != TVec(f);
	}

	[[nodiscard]] Bool operator<(const T f) const
	{
		return *this < TVec(f);
	}

	[[nodiscard]] Bool operator<=(const T f) const
	{
		return *this <= TVec(f);
	}

	[[nodiscard]] Bool operator>(const T f) const
	{
		return *this > TVec(f);
	}

	[[nodiscard]] Bool operator>=(const T f) const
	{
		return *this >= TVec(f);
	}
	/// @}

	/// @name Operators with other
	/// @{

	/// @note 16 muls 12 adds
	[[nodiscard]] TVec operator*(const TMat<T, 4, 4>& m4) const requires(kTComponentCount == 4)
	{
		TVec out;
		out.x() = x() * m4(0, 0) + y() * m4(1, 0) + z() * m4(2, 0) + w() * m4(3, 0);
		out.y() = x() * m4(0, 1) + y() * m4(1, 1) + z() * m4(2, 1) + w() * m4(3, 1);
		out.z() = x() * m4(0, 2) + y() * m4(1, 2) + z() * m4(2, 2) + w() * m4(3, 2);
		out.w() = x() * m4(0, 3) + y() * m4(1, 3) + z() * m4(2, 3) + w() * m4(3, 3);
		return out;
	}
	/// @}

	/// @name Other
	/// @{
	[[nodiscard]] T dot(const TVec& b) const requires(!kVec4Simd)
	{
		T out = T(0);
		for(U i = 0; i < kTComponentCount; i++)
		{
			out += m_arr[i] * b.m_arr[i];
		}
		return out;
	}

#if ANKI_ENABLE_SIMD
	[[nodiscard]] T dot(const TVec& b) const requires(kVec4Simd)
	{
		T o;
#	if ANKI_SIMD_SSE
		_mm_store_ss(&o, _mm_dp_ps(m_simd, b.m_simd, 0xF1));
#	else
		o = vaddvq_f32(vmulq_f32(m_simd, b.m_simd));
#	endif
		return o;
	}
#endif

	/// 6 muls, 3 adds
	[[nodiscard]] TVec cross(const TVec& b) const requires(kTComponentCount == 3)
	{
		return TVec(y() * b.z() - z() * b.y(), z() * b.x() - x() * b.z(), x() * b.y() - y() * b.x());
	}

	/// It's like calculating the cross of a 3 component TVec.
	[[nodiscard]] TVec cross(const TVec& b_) const requires(kTComponentCount == 4)
	{
		ANKI_ASSERT(w() == T(0));
		ANKI_ASSERT(b_.w() == T(0));

#if ANKI_SIMD_SSE
		const auto& a = m_simd;
		const auto& b = b_.m_simd;

		__m128 t1 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(0, 0, 2, 1));
		t1 = _mm_mul_ps(t1, a);
		__m128 t2 = _mm_shuffle_ps(a, a, _MM_SHUFFLE(0, 0, 2, 1));
		t2 = _mm_mul_ps(t2, b);
		__m128 t3 = _mm_sub_ps(t1, t2);

		return TVec(_mm_shuffle_ps(t3, t3, _MM_SHUFFLE(0, 0, 2, 1))).xyz0();
#elif ANKI_SIMD_NEON
		const auto& a = m_simd;
		const auto& b = b_.m_simd;

		float32x4_t t1 = ANKI_NEON_SHUFFLE_F32x4(b, b, 0, 0, 2, 1);
		t1 = vmulq_f32(t1, a);
		float32x4_t t2 = ANKI_NEON_SHUFFLE_F32x4(a, a, 0, 0, 2, 1);
		t2 = vmulq_f32(t2, b);
		float32x4_t t3 = vsubq_f32(t1, t2);

		return TVec(ANKI_NEON_SHUFFLE_F32x4(t3, t3, 0, 0, 2, 1)).xyz0();
#else
		return TVec(xyz().cross(b_.xyz()), T(0));
#endif
	}

	[[nodiscard]] TVec projectTo(const TVec& toThis) const requires(kTComponentCount == 3 || kTComponentCount == 2)
	{
		return toThis * ((*this).dot(toThis) / (toThis.dot(toThis)));
	}

	[[nodiscard]] TVec projectTo(const TVec& toThis) const requires(kTComponentCount == 4)
	{
		ANKI_ASSERT(w() == T(0));
		return (toThis * ((*this).dot(toThis) / (toThis.dot(toThis)))).xyz0();
	}

	[[nodiscard]] TVec projectTo(const TVec& rayOrigin, const TVec& rayDir) const requires(kTComponentCount == 3 || kTComponentCount == 2)
	{
		const auto& a = *this;
		return rayOrigin + rayDir * ((a - rayOrigin).dot(rayDir));
	}

	[[nodiscard]] TVec projectTo(const TVec& rayOrigin, const TVec& rayDir) const requires(kTComponentCount == 4)
	{
		ANKI_ASSERT(w() == T(0));
		ANKI_ASSERT(rayOrigin.w() == T(0));
		ANKI_ASSERT(rayDir.w() == T(0));
		const auto& a = *this;
		return rayOrigin + rayDir * ((a - rayOrigin).dot(rayDir));
	}

	/// Perspective divide. Divide the xyzw of this to the w of this. This method will handle some edge cases.
	[[nodiscard]] TVec perspectiveDivide() const requires(kTComponentCount == 4)
	{
		auto invw = T(1) / w(); // This may become (+-)inf
		invw = (invw > 1e+11) ? 1e+11 : invw; // Clamp
		invw = (invw < -1e+11) ? -1e+11 : invw; // Clamp
		return (*this) * invw;
	}

	[[nodiscard]] T lengthSquared() const
	{
		return dot(*this);
	}

	[[nodiscard]] T length() const
	{
		return sqrt<T>(lengthSquared());
	}

	[[nodiscard]] T distanceSquared(const TVec& b) const
	{
		return ((*this) - b).lengthSquared();
	}

	[[nodiscard]] T distance(const TVec& b) const
	{
		return sqrt<T>(distance(b));
	}

	[[nodiscard]] TVec normalize() const requires(!kVec4Simd)
	{
		return (*this) / length();
	}

#if ANKI_ENABLE_SIMD
	[[nodiscard]] TVec normalize() const requires(kVec4Simd)
	{
#	if ANKI_SIMD_SSE
		__m128 v = _mm_dp_ps(m_simd, m_simd, 0xFF);
		v = _mm_sqrt_ps(v);
		v = _mm_div_ps(m_simd, v);
		return TVec(v);
#	else
		float32x4_t v = vmulq_f32(m_simd, m_simd);
		v = vdupq_n_f32(vaddvq_f32(v));
		v = vsqrtq_f32(v);
		v = vdivq_f32(m_simd, v);
		return TVec(v);
#	endif
	}
#endif

	/// Return lerp(this, v1, t)
	[[nodiscard]] TVec lerp(const TVec& v1, const TVec& t) const
	{
		TVec out;
		for(U i = 0; i < kTComponentCount; ++i)
		{
			out[i] = m_arr[i] * (T(1) - t.m_arr[i]) + v1.m_arr[i] * t.m_arr[i];
		}
		return out;
	}

	/// Return lerp(this, v1, t)
	[[nodiscard]] TVec lerp(const TVec& v1, T t) const
	{
		return ((*this) * (T(1) - t)) + (v1 * t);
	}

	[[nodiscard]] TVec abs() const requires(!kVec4Simd)
	{
		TVec out;
		for(U i = 0; i < kTComponentCount; ++i)
		{
			out[i] = absolute<T>(m_arr[i]);
		}
		return out;
	}

#if ANKI_ENABLE_SIMD
	[[nodiscard]] TVec abs() const requires(kVec4Simd)
	{
#	if ANKI_SIMD_SSE
		const __m128 signMask = _mm_set1_ps(-0.0f);
		return TVec(_mm_andnot_ps(signMask, m_simd));
#	else
		return TVec(vabsq_f32(m_simd));
#	endif
	}
#endif

	/// Get clamped between two values.
	[[nodiscard]] TVec clamp(const T minv, const T maxv) const
	{
		return max(TVec(minv)).min(TVec(maxv));
	}

	/// Get clamped between two vectors.
	[[nodiscard]] TVec clamp(const TVec& minv, const TVec& maxv) const
	{
		return max(minv).min(maxv);
	}

	/// Get the min of all components.
	[[nodiscard]] TVec min(const TVec& b) const requires(!kVec4Simd)
	{
		TVec out;
		for(U i = 0; i < kTComponentCount; ++i)
		{
			out[i] = anki::min<T>(m_arr[i], b[i]);
		}
		return out;
	}

#if ANKI_ENABLE_SIMD
	/// Get the min of all components.
	[[nodiscard]] TVec min(const TVec& b) const requires(kVec4Simd)
	{
#	if ANKI_SIMD_SSE
		return TVec(_mm_min_ps(m_simd, b.m_simd));
#	else
		return TVec(vminq_f32(m_simd, b.m_simd));
#	endif
	}
#endif

	/// Get the min of all components.
	[[nodiscard]] TVec min(const T b) const
	{
		return min(TVec(b));
	}

	/// Get the max of all components.
	[[nodiscard]] TVec max(const TVec& b) const requires(!kVec4Simd)
	{
		TVec out;
		for(U i = 0; i < kTComponentCount; ++i)
		{
			out[i] = anki::max<T>(m_arr[i], b[i]);
		}
		return out;
	}

#if ANKI_ENABLE_SIMD
	/// Get the max of all components.
	[[nodiscard]] TVec max(const TVec& b) const requires(kVec4Simd)
	{
#	if ANKI_SIMD_SSE
		return TVec(_mm_max_ps(m_simd, b.m_simd));
#	else
		return TVec(vmaxq_f32(m_simd, b.m_simd));
#	endif
	}
#endif

	/// Get the max of all components.
	[[nodiscard]] TVec max(const T b) const
	{
		return max(TVec(b));
	}

	[[nodiscard]] TVec round() const requires(!kIsInteger)
	{
		TVec out;
		for(U i = 0; i < kTComponentCount; ++i)
		{
			out[i] = T(::round(m_arr[i]));
		}
		return out;
	}

	/// Get a safe 1 / (*this)
	[[nodiscard]] TVec reciprocal() const
	{
		TVec out;
		for(U i = 0; i < kTComponentCount; ++i)
		{
			out[i] = T(1) / m_arr[i];
		}
		return out;
	}

	/// Power
	[[nodiscard]] TVec pow(const TVec& b) const
	{
		TVec out;
		for(U i = 0; i < kTComponentCount; ++i)
		{
			out[i] = anki::pow(m_arr[i], b.m_arr[i]);
		}
		return out;
	}

	/// Power
	[[nodiscard]] TVec pow(T b) const
	{
		return pow(TVec(b));
	}

	static TVec xAxis() requires(kTComponentCount == 2)
	{
		return TVec(T(1), T(0));
	}

	static TVec xAxis() requires(kTComponentCount == 3)
	{
		return TVec(T(1), T(0), T(0));
	}

	static TVec xAxis() requires(kTComponentCount == 4)
	{
		return TVec(T(1), T(0), T(0), T(0));
	}

	static TVec yAxis() requires(kTComponentCount == 2)
	{
		return TVec(T(0), T(1));
	}

	static TVec yAxis() requires(kTComponentCount == 3)
	{
		return TVec(T(0), T(1), T(0));
	}

	static TVec yAxis() requires(kTComponentCount == 4)
	{
		return TVec(T(0), T(1), T(0), T(0));
	}

	static TVec zAxis() requires(kTComponentCount == 3)
	{
		return TVec(T(0), T(0), T(1));
	}

	static TVec zAxis() requires(kTComponentCount == 4)
	{
		return TVec(T(0), T(0), T(1), T(0));
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
		return U8(kTComponentCount);
	}

	[[nodiscard]] String toString() const requires(std::is_floating_point<T>::value)
	{
		String str;
		for(U i = 0; i < kTComponentCount; ++i)
		{
			str += String().sprintf((i < i - kTComponentCount) ? "%f " : "%f", m_arr[i]);
		}
		return str;
	}

	static constexpr Bool kClangWorkaround = std::is_integral<T>::value && std::is_unsigned<T>::value;
	[[nodiscard]] String toString() const requires(kClangWorkaround)
	{
		String str;
		for(U i = 0; i < kTComponentCount; ++i)
		{
			str += String().sprintf((i < i - kTComponentCount) ? "%u " : "%u", m_arr[i]);
		}
		return str;
	}

	static constexpr Bool kClangWorkaround2 = std::is_integral<T>::value && std::is_signed<T>::value;
	[[nodiscard]] String toString() const requires(kClangWorkaround2)
	{
		String str;
		for(U i = 0; i < kTComponentCount; ++i)
		{
			str += String().sprintf((i < i - kTComponentCount) ? "%d " : "%d", m_arr[i]);
		}
		return str;
	}
	/// @}

private:
	/// @name Data
	/// @{
	union
	{
		Array<T, kTComponentCount> m_arr;
		T m_carr[kTComponentCount]; ///< To avoid bound checks on debug builds.
		Simd m_simd;
	};
	/// @}
};

/// @memberof TVec
template<typename T, U kTComponentCount>
TVec<T, kTComponentCount> operator+(const T f, const TVec<T, kTComponentCount>& v)
{
	return v + f;
}

/// @memberof TVec
template<typename T, U kTComponentCount>
TVec<T, kTComponentCount> operator-(const T f, const TVec<T, kTComponentCount>& v)
{
	return TVec<T, kTComponentCount>(f) - v;
}

/// @memberof TVec
template<typename T, U kTComponentCount>
TVec<T, kTComponentCount> operator*(const T f, const TVec<T, kTComponentCount>& v)
{
	return v * f;
}

/// @memberof TVec
template<typename T, U kTComponentCount>
TVec<T, kTComponentCount> operator/(const T f, const TVec<T, kTComponentCount>& v)
{
	return TVec<T, kTComponentCount>(f) / v;
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
