// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_MATH_FUNCTIONS_H
#define ANKI_MATH_FUNCTIONS_H

#include "anki/util/StdTypes.h"
#include <cmath>
#include <cstdlib>

namespace anki {

/// @addtogroup math
/// @{

//==============================================================================
// Math constants                                                              =
//==============================================================================

template<typename Scalar>
constexpr Scalar getPi();

template<>
inline constexpr F32 getPi<F32>()
{
	return 3.14159265358979323846f;
}

template<>
inline constexpr F64 getPi<F64>()
{
	return 3.14159265358979323846;
}

template<typename Scalar>
constexpr Scalar getEpsilon()
{
	static_assert(1, "Shouldn't instantiate");
	return Scalar(0);
}

template<>
constexpr F32 getEpsilon<F32>()
{
	return 1.0e-6f;
}

template<>
constexpr F64 getEpsilon<F64>()
{
	return 1.0e-6;
}

//==============================================================================
// Math functions                                                              =
//==============================================================================

template<typename T>
inline T fabs(const T x)
{
	return std::fabs(x);
}

template<typename T>
inline T sin(const T rad)
{
	return std::sin(rad);
}

template<typename T>
inline T cos(const T rad)
{
	return std::cos(rad);
}

template<typename T>
inline T tan(const T rad)
{
	return std::tan(rad);
}

template<typename T>
inline T acos(const T x)
{
	return std::acos(x);
}

template<typename T>
inline T asin(const T x)
{
	return std::asin(x);
}

template<typename T>
inline T atan(const T x)
{
	return std::atan(x);
}

template<typename T>
inline T atan2(const T x, const T y)
{
	return std::atan2(x, y);
}

void sinCos(const F32 a, F32& sina, F32& cosa);
void sinCos(const F64 a, F64& sina, F64& cosa);

template<typename T>
inline T sqrt(const T x)
{
	return std::sqrt(x);
}

template<typename T>
inline T fract(const T x)
{
	return x - std::floor(x);
}

template<typename T>
inline T mod(const T x, const T y)
{
	return x - y * std::floor(x / y);
}       

//==============================================================================
// Other math functions                                                        =
//==============================================================================

template<typename T>
inline T abs(const T f)
{ 
	return std::fabs(f);
}

#define ANKI_SPECIALIZE_ABS_INT(type_) \
	template<> \
	inline type_ abs(const type_ f) \
	{ \
		return std::abs(f); \
	}

ANKI_SPECIALIZE_ABS_INT(I8)
ANKI_SPECIALIZE_ABS_INT(I16)
ANKI_SPECIALIZE_ABS_INT(I32)
ANKI_SPECIALIZE_ABS_INT(I64)

#undef ANKI_SPECIALIZE_ABS_INT

template<typename T>
inline Bool isZero(const T f)
{
	return abs<T>(f) < getEpsilon<T>();
}

#define ANKI_SPECIALIZE_IS_ZERO_INT(type_) \
	template<> \
	inline Bool isZero(const type_ x) \
	{ \
		return x == type_(0); \
	}

ANKI_SPECIALIZE_IS_ZERO_INT(I8)
ANKI_SPECIALIZE_IS_ZERO_INT(I16)
ANKI_SPECIALIZE_IS_ZERO_INT(I32)
ANKI_SPECIALIZE_IS_ZERO_INT(I64)
ANKI_SPECIALIZE_IS_ZERO_INT(U8)
ANKI_SPECIALIZE_IS_ZERO_INT(U16)
ANKI_SPECIALIZE_IS_ZERO_INT(U32)
ANKI_SPECIALIZE_IS_ZERO_INT(U64)

#undef ANKI_SPECIALIZE_IS_ZERO_INT

template<typename T>
inline T toRad(const T degrees)
{
	return degrees * (getPi<T>() / T(180));
}

template<typename T>
inline T toDegrees(const T rad)
{
	return rad * (T(180) / getPi<T>());
}

//==============================================================================
// Interpolation                                                               =
//==============================================================================

/// Linear interpolation between values
/// @param[in] from Starting value
/// @param[in] to Ending value
/// @param[in] u The percentage from the from "from" value. Values
///              from [0.0, 1.0]
template<typename Type>
static Type linearInterpolate(const Type& from, const Type& to, F32 u)
{
	return from * (1.0 - u) + to * u;
}

/// Cosine interpolation
/// @param[in] from Starting value
/// @param[in] to Ending value
/// @param[in] u The percentage from the from "from" value. Values
///              from [0.0, 1.0]
template<typename Type>
static Type cosInterpolate(const Type& from, const Type& to, F32 u)
{
	F32 u2 = (1.0 - cos<F32>(u * getPi<F32>())) / 2.0;
	return from * (1.0 - u2) + to * u2;
}

/// Cubic interpolation
/// @param[in] a Point a
/// @param[in] b Point b
/// @param[in] c Point c
/// @param[in] d Point d
/// @param[in] u The percentage from the from b point to d point. Value
///              from [0.0, 1.0]
template<typename Type>
static Type cubicInterpolate(
	const Type& a, const Type& b, const Type& c, 
	const Type& d, F32 u)
{
	F32 u2 = u * u;
	Type a0 = d - c - a + b;
	Type a1 = a - b - a0;
	Type a2 = c - a;
	Type a3 = b;

	return(a0 * u * u2 + a1 * u2 + a2 * u + a3);
}

/// @}

} // end namespace anki

#endif
