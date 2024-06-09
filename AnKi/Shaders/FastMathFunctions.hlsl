// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Common.hlsl>

/// Sin approximation: https://www.desmos.com/calculator/svgcjfskne
F32 fastSin(F32 x)
{
	const F32 k2Pi = 2.0 * kPi;
	const F32 kPiOver2 = kPi / 2.0;

	x = (x + kPiOver2) / (k2Pi) + 0.75;
	x = frac(x);
	x = x * 2.0 - 1.0;
	x = x * abs(x) - x;
	x *= 4.0;
	return x;
}

/// Cos approximation
F32 fastCos(F32 x)
{
	return fastSin(x + kPi / 2.0);
}

F32 fastSqrt(F32 x)
{
	return asfloat(0x1FBD1DF5 + (asint(x) >> 1));
}

F32 fastAcos(F32 x)
{
	F32 res = -0.156583f * abs(x) + kPi / 2.0f;
	res *= fastSqrt(1.0f - abs(x));
	return (x > 0.0f) ? res : kPi - res;
}
