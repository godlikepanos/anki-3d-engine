#ifndef ANKI_MATH_MATH_SIMD_H
#define ANKI_MATH_MATH_SIMD_H

#include "anki/Config.h"

#if ANKI_MATH_SIMD == ANKI_MATH_SIMD_SSE
#	include <smmintrin.h>
#elif ANKI_MATH_SIMD == ANKI_MATH_SIMD_NEON
#	error "Not implemented yet"
#elif ANKI_MATH_SIMD == ANKI_MATH_SIMD_NONE
#	define ANKI_DUMMY_DUMMY_DUMMY 1
#else
#	error "See file"
#endif

#endif
