// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StdTypes.h>

#if ANKI_SIMD == ANKI_SIMD_SSE
#	include <smmintrin.h>
#elif ANKI_SIMD == ANKI_SIMD_NEON
#	include <arm_neon.h>
#elif ANKI_SIMD == ANKI_SIMD_NONE
#	define ANKI_DUMMY_DUMMY_DUMMY 1
#else
#	error "See file"
#endif

namespace anki
{

/// Template class XXX
template<typename T, U N>
class MathSimd
{
public:
	using Type = T[N];
};

#if ANKI_SIMD == ANKI_SIMD_SSE
// Specialize for F32
template<>
class MathSimd<F32, 4>
{
public:
	using Type = __m128;
};
#elif ANKI_SIMD == ANKI_SIMD_NEON
// Specialize for F32
template<>
class MathSimd<F32, 4>
{
public:
	using Type = float32x4_t;
};
#endif

} // end namespace anki