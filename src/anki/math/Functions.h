// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StdTypes.h>
#include <cmath>
#include <cstdlib>

namespace anki
{

/// @addtogroup math
/// @{

/// Just PI.
const F32 PI = 3.14159265358979323846f;

/// Floating point epsilon.
const F32 EPSILON = 1.0e-6f;

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
	ANKI_ASSERT(x >= T(-1) && x <= T(1));
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

/// The same as abs/fabs. For ints and floats.
template<typename T>
T absolute(const T f);

#define ANKI_SPECIALIZE_ABS(type_) \
	template<> \
	inline type_ absolute(const type_ f) \
	{ \
		return (f < type_(0)) ? -f : f; \
	}

ANKI_SPECIALIZE_ABS(I8)
ANKI_SPECIALIZE_ABS(I16)
ANKI_SPECIALIZE_ABS(I32)
ANKI_SPECIALIZE_ABS(I64)
ANKI_SPECIALIZE_ABS(F32)
ANKI_SPECIALIZE_ABS(F64)

#undef ANKI_SPECIALIZE_ABS

template<typename T>
inline Bool isZero(const T f)
{
	return absolute<T>(f) < EPSILON;
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
	return degrees * (PI / T(180));
}

template<typename T>
inline T toDegrees(const T rad)
{
	return rad * (T(180) / PI);
}

/// Linear interpolation between values
/// @param[in] from Starting value
/// @param[in] to Ending value
/// @param[in] u The percentage from the from "from" value. Values from [0.0, 1.0]
template<typename Type>
inline Type linearInterpolate(const Type& from, const Type& to, F32 u)
{
	return from * (1.0f - u) + to * u;
}

/// Cosine interpolation
/// @param[in] from Starting value
/// @param[in] to Ending value
/// @param[in] u The percentage from the from "from" value. Values from [0.0, 1.0]
template<typename Type>
inline Type cosInterpolate(const Type& from, const Type& to, F32 u)
{
	F32 u2 = (1.0f - cos<Type>(u * PI)) / 2.0f;
	return from * (1.0f - u2) + to * u2;
}

/// Cubic interpolation
/// @param[in] a Point a
/// @param[in] b Point b
/// @param[in] c Point c
/// @param[in] d Point d
/// @param[in] u The percentage from the from b point to d point. Value from [0.0, 1.0]
template<typename Type>
inline Type cubicInterpolate(const Type& a, const Type& b, const Type& c, const Type& d, F32 u)
{
	F32 u2 = u * u;
	Type a0 = d - c - a + b;
	Type a1 = a - b - a0;
	Type a2 = c - a;
	Type a3 = b;

	return (a0 * u * u2 + a1 * u2 + a2 * u + a3);
}

/// Pack 4 color components to R10G10B10A2 SNORM format.
inline U32 packColorToR10G10B10A2SNorm(F32 r, F32 g, F32 b, F32 a)
{
	union SignedR10G10B10A10
	{
		struct
		{
			I m_x : 10;
			I m_y : 10;
			I m_z : 10;
			I m_w : 2;
		} m_unpacked;
		U32 m_packed;
	};

	SignedR10G10B10A10 out;
	out.m_unpacked.m_x = I(round(r * 511.0f));
	out.m_unpacked.m_y = I(round(g * 511.0f));
	out.m_unpacked.m_z = I(round(b * 511.0f));
	out.m_unpacked.m_w = I(round(a * 1.0f));

	return out.m_packed;
}
/// @}

} // end namespace anki
