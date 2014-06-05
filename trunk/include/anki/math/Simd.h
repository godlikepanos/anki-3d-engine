// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_MATH_MATH_SIMD_H
#define ANKI_MATH_MATH_SIMD_H

#include "anki/Config.h"

#if ANKI_SIMD == ANKI_SIMD_SSE
#	include <smmintrin.h>
#elif ANKI_SIMD == ANKI_SIMD_NEON
#	include <arm_neon.h>
#elif ANKI_SIMD == ANKI_SIMD_NONE
#	define ANKI_DUMMY_DUMMY_DUMMY 1
#else
#	error "See file"
#endif

#endif
