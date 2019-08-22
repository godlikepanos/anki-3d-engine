// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Functions.h>
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

template<typename T, ANKI_ENABLE(std::is_floating_point<T>::value)>
inline T sqrt(const T x)
{
	return std::sqrt(x);
}

template<typename T, ANKI_ENABLE(std::is_integral<T>::value)>
inline T sqrt(const T x)
{
	return T(std::sqrt(F64(x)));
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

/// Like GLSL's mix.
template<typename T, typename Y>
inline T mix(T x, T y, Y factor)
{
	return x * (T(1) - factor) + y * factor;
}

/// Like GLSL's modf
template<typename T>
inline T modf(T x, T& intPart)
{
	return std::modf(x, &intPart);
}

/// The same as abs/fabs. For ints and floats.
template<typename T,
	typename std::enable_if<std::is_floating_point<T>::value
								|| (std::is_integral<T>::value && std::is_signed<T>::value),
		int>::type = 0>
inline T absolute(const T f)
{
	return (f < T(0)) ? -f : f;
}

template<typename T>
inline T pow(const T x, const T power)
{
	return T(std::pow(x, power));
}

template<typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
inline Bool isZero(const T f, const T e = EPSILON)
{
	return absolute<T>(f) < e;
}

template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
inline Bool isZero(const T f)
{
	return f == 0;
}

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
	return from * Type(1.0f - u) + to * Type(u);
}

/// Cosine interpolation
/// @param[in] from Starting value
/// @param[in] to Ending value
/// @param[in] u The percentage from the from "from" value. Values from [0.0, 1.0]
template<typename Type>
inline Type cosInterpolate(const Type& from, const Type& to, F32 u)
{
	const F32 u2 = (1.0f - cos<Type>(u * PI)) / 2.0f;
	return from * Type(1.0f - u2) + to * Type(u2);
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
			I32 m_x : 10;
			I32 m_y : 10;
			I32 m_z : 10;
			I32 m_w : 2;
		} m_unpacked;
		U32 m_packed;
	};

#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wconversion"
#endif

	SignedR10G10B10A10 out;
	out.m_unpacked.m_x = I32(round(r * 511.0f));
	out.m_unpacked.m_y = I32(round(g * 511.0f));
	out.m_unpacked.m_z = I32(round(b * 511.0f));
	out.m_unpacked.m_w = I32(round(a * 1.0f));

#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic pop
#endif

	return out.m_packed;
}

/// Compute the abs triangle area.
template<typename TVec>
inline F32 computeTriangleArea(const TVec& a, const TVec& b, const TVec& c)
{
	const TVec ab = b - a;
	const TVec ac = c - a;
	const F32 area = ab.cross(ac).getLength() / 2.0f;
	return absolute(area);
}
/// @}

} // end namespace anki
