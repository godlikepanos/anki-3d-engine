// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Common.hlsl>

/// Sin approximation: https://www.desmos.com/calculator/svgcjfskne
template<typename T>
T fastSin(T x)
{
	x = (x + kHalfPi) / (k2Pi) + T(0.75);
	x = frac(x);
	x = x * T(2) - T(1);
	x = x * abs(x) - x;
	x *= T(4);
	return x;
}

/// Cos approximation
template<typename T>
T fastCos(T x)
{
	return fastSin<T>(x + T(kHalfPi));
}

F32 fastSqrt(F32 x)
{
	return asfloat(0x1FBD1DF5 + (asint(x) >> 1));
}

template<typename T>
T fastAcos(T x)
{
	T res = T(-0.156583) * abs(x) + T(kHalfPi);
	res *= fastSqrt(T(1) - abs(x));
	return (x > T(0)) ? res : T(kPi) - res;
}
