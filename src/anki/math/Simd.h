// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/Config.h>

#if ANKI_SIMD == ANKI_SIMD_SSE
#include <smmintrin.h>
#elif ANKI_SIMD == ANKI_SIMD_NEON
#include <arm_neon.h>
#elif ANKI_SIMD == ANKI_SIMD_NONE
#define ANKI_DUMMY_DUMMY_DUMMY 1
#else
#error "See file"
#endif
