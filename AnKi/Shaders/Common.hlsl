// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// This file contains common code for all shaders. It's optional but it's recomended to include it

#pragma once

#include <AnKi/Shaders/Include/Common.h>

template<typename T>
T uvToNdc(T x)
{
	return x * 2.0f - 1.0f;
}

template<typename T>
T ndcToUv(T x)
{
	return x * 0.5f + 0.5f;
}

// Define min3, max3, min4, max4 functions

#define DEFINE_COMPARISON(func, scalarType, vectorType) \
	scalarType func##4(vectorType##4 v) \
	{ \
		return func(v.x, func(v.y, func(v.z, v.w))); \
	} \
	scalarType func##4(scalarType x, scalarType y, scalarType z, scalarType w) \
	{ \
		return func(x, func(y, func(z, w))); \
	} \
	scalarType func##3(vectorType##3 v) \
	{ \
		return func(v.x, func(v.y, v.z)); \
	} \
	scalarType func##3(scalarType x, scalarType y, scalarType z) \
	{ \
		return func(x, func(y, z)); \
	}

#define DEFINE_COMPARISON2(func) \
	DEFINE_COMPARISON(func, F32, Vec) \
	DEFINE_COMPARISON(func, I32, IVec) \
	DEFINE_COMPARISON(func, U32, UVec)

DEFINE_COMPARISON2(min)
DEFINE_COMPARISON2(max)

#undef DEFINE_COMPARISON2
#undef DEFINE_COMPARISON
