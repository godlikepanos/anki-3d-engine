// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/math/CommonIncludes.h>

namespace anki
{

/// @addtogroup math
/// @{

/// Template struct that gives the type of the TVec4 SIMD
template<typename T>
class TVec4Simd
{
public:
	using Type = Array<T, 4>;
};

#if ANKI_SIMD == ANKI_SIMD_SSE
// Specialize for F32
template<>
class TVec4Simd<F32>
{
public:
	using Type = __m128;
};
#elif ANKI_SIMD == ANKI_SIMD_NEON
// Specialize for F32
template<>
class TVec4Simd<F32>
{
public:
	using Type = float32x4_t;
};
#endif

/// 4D vector. SIMD optimized
template<typename T>
class alignas(16) TVec4 : public TVec<T, 4, typename TVec4Simd<T>::Type, TVec4<T>>
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
	using Base = TVec<T, 4, typename TVec4Simd<T>::Type, TVec4<T>>;

	using Base::x;
	using Base::y;
	using Base::z;
	using Base::w;
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

	TVec4(const TVec3<T>& v, const T w_)
		: Base(v.x(), v.y(), v.z(), w_)
	{
	}
	/// @}

	/// @name Operators with same
	/// @{

	/// It's like calculating the cross of a TVec3
	ANKI_USE_RESULT TVec4 cross(const TVec4& b) const
	{
		ANKI_ASSERT(isZero<T>(Base::w()));
		ANKI_ASSERT(isZero<T>(b.w()));
		return TVec4(Base::xyz().cross(b.xyz()), static_cast<T>(0));
	}

	ANKI_USE_RESULT TVec4 projectTo(const TVec4& toThis) const
	{
		ANKI_ASSERT(w() == T(0));
		return (toThis * ((*this).dot(toThis) / (toThis.dot(toThis)))).xyz0();
	}

	ANKI_USE_RESULT TVec4 projectTo(const TVec4& rayOrigin, const TVec4& rayDir) const
	{
		const auto& a = *this;
		return rayOrigin + rayDir * ((a - rayOrigin).dot(rayDir));
	}
	/// @{

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

	/// Perspective divide. Divide the xyzw of this to the w of this. This method will handle some edge cases.
	ANKI_USE_RESULT TVec4 perspectiveDivide() const
	{
		auto invw = T(1) / w(); // This may become (+-)inf
		invw = (invw > 1e+11) ? 1e+11 : invw; // Clamp
		invw = (invw < -1e+11) ? -1e+11 : invw; // Clamp
		return (*this) * invw;
	}
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
TVec4<F32>& TVec4<F32>::Base::operator=(const TVec4<F32>& b);

template<>
TVec4<F32> TVec4<F32>::Base::operator+(const TVec4<F32>& b) const;

template<>
TVec4<F32>& TVec4<F32>::Base::operator+=(const TVec4<F32>& b);

template<>
TVec4<F32> TVec4<F32>::Base::operator-(const TVec4<F32>& b) const;

template<>
TVec4<F32>& TVec4<F32>::Base::operator-=(const TVec4<F32>& b);

template<>
TVec4<F32> TVec4<F32>::Base::operator*(const TVec4<F32>& b) const;

template<>
TVec4<F32>& TVec4<F32>::Base::operator*=(const TVec4<F32>& b);

template<>
TVec4<F32> TVec4<F32>::Base::operator/(const TVec4<F32>& b) const;

template<>
TVec4<F32>& TVec4<F32>::Base::operator/=(const TVec4<F32>& b);

template<>
F32 TVec4<F32>::Base::dot(const TVec4<F32>& b) const;

template<>
TVec4<F32> TVec4<F32>::Base::getNormalized() const;

template<>
void TVec4<F32>::Base::normalize();

template<>
TVec4<F32> TVec4<F32>::cross(const TVec4<F32>& b) const;

template<>
TVec4<F32> TVec4<F32>::Base::getAbs() const;

template<>
F32 TVec4<F32>::Base::getLengthSquared() const;

template<>
TVec4<F32> TVec4<F32>::Base::operator-() const;

#elif ANKI_SIMD == ANKI_SIMD_NEON

#error "TODO"

#endif

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
