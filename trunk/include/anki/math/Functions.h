#ifndef ANKI_MATH_FUNCTIONS_H
#define ANKI_MATH_FUNCTIONS_H

#include "anki/util/StdTypes.h"
#include <cmath>

namespace anki {

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
constexpr Scalar getEpsilon();

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

//==============================================================================
// Other math functions                                                        =
//==============================================================================

template<typename T>
inline Bool isZero(const T f)
{
	return fabs(f) < getEpsilon<T>();
}

template<typename T>
inline T toRad(const T degrees)
{
	return degrees * (getPi<T>() / T(180.0));
}

template<typename T>
inline T toDegrees(const T rad)
{
	return rad * (T(180.0) / getPi<T>());
}

} // end namespace anki

#endif
