// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_MATH_VEC4_H
#define ANKI_MATH_VEC4_H

#include "anki/math/CommonIncludes.h"

namespace anki {

/// @addtogroup math
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
class alignas(16) TVec4: 
	public TVec<T, 4, typename TVec4Simd<T>::Type, TVec4<T>>
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
	explicit TVec4()
		: Base()
	{}

	TVec4(const TVec4& b)
		: Base(b)
	{}

	explicit TVec4(const T x_, const T y_, const T z_, const T w_)
		: Base(x_, y_, z_, w_)
	{}

	explicit TVec4(const T f)
		: Base(f)
	{}

	explicit TVec4(const T arr[])
		: Base(arr)
	{}

	explicit TVec4(const typename Base::Simd& simd)
		: Base(simd)
	{}

	explicit TVec4(const TVec2<T>& v, const T z_, const T w_)
		: Base(v.x(), v.y(), z_, w_)
	{}

	explicit TVec4(const TVec3<T>& v, const T w_)
		: Base(v.x(), v.y(), v.z(), w_)
	{}
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

#elif ANKI_SIMD == ANKI_SIMD_NEON

#	error "TODO"

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
