// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StdTypes.h>

#if ANKI_SIMD_SSE
#	include <smmintrin.h>
#elif ANKI_SIMD_NEON
#	include <arm_neon.h>
#elif !ANKI_SIMD_NONE
#	error "See file"
#endif

namespace anki
{

/// Template class that holds SIMD info for the math classes.
template<typename T, U N>
class MathSimd
{
public:
	using Type = T[N];
	static constexpr U ALIGNMENT = alignof(T);
};

#if ANKI_SIMD_SSE
// Specialize for F32
template<>
class MathSimd<F32, 4>
{
public:
	using Type = __m128;
	static constexpr U ALIGNMENT = 16;
};
#elif ANKI_SIMD_NEON
// Specialize for F32
template<>
class MathSimd<F32, 4>
{
public:
	using Type = float32x4_t;
	static constexpr U ALIGNMENT = 16;
};
#endif

} // end namespace anki
