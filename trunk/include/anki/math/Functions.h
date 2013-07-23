#ifndef ANKI_MATH_FUNCTIONS_H
#define ANKI_MATH_FUNCTIONS_H

#include "anki/util/StdTypes.h"
#include <cmath>

namespace anki {

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

template<typename T>
inline Bool isZero(const T f)
{
	return fabs(f) < getEpsilon<T>();
}

inline F32 toRad(const F32 degrees)
{
	return degrees * (getPi<F32>() / 180.0f);
}

inline F64 toRad(const F64 degrees)
{
	return degrees * (getPi<F64>() / 180.0);
}

inline F32 toDegrees(const F32 rad)
{
	return rad * (180.0f / getPi<F32>());
}

inline F64 toDegrees(const F64 rad)
{
	return rad * (180.0 / getPi<F64>());
}

inline F32 sin(const F32 rad)
{
	return ::sin(rad);
}

inline F64 sin(const F64 rad)
{
	return ::sin(rad);
}

inline F32 cos(const F32 rad)
{
	return ::cos(rad);
}

inline F64 cos(const F64 rad)
{
	return ::cos(rad);
}

void sinCos(const F32 a, F32& sina, F32& cosa);
void sinCos(const F64 a, F64& sina, F64& cosa);

} // end namespace anki

#endif
