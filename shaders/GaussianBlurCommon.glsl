// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_GAUSSIAN_BLUR_COMMON_GLSL
#define ANKI_SHADERS_GAUSSIAN_BLUR_COMMON_GLSL

#include "shaders/Common.glsl"

#if KERNEL_SIZE == 3
const uint STEP_COUNT = 1u;
const float WEIGHTS[STEP_COUNT + 1u] = {0.361069, 0.319466};
#elif KERNEL_SIZE == 5
const uint STEP_COUNT = 2u;
const float WEIGHTS[STEP_COUNT + 1u] = {0.250301, 0.221461, 0.153388};
#elif KERNEL_SIZE == 7
const uint STEP_COUNT = 3u;
const float WEIGHTS[STEP_COUNT + 1u] = {0.214607, 0.189879, 0.131514, 0.071303};
#elif KERNEL_SIZE == 9
const uint STEP_COUNT = 4u;
const float WEIGHTS[STEP_COUNT + 1u] = {0.20236, 0.179044, 0.124009, 0.067234, 0.028532};
#elif KERNEL_SIZE == 11
const uint STEP_COUNT = 5u;
const float WEIGHTS[STEP_COUNT + 1u] = {0.198596, 0.175713, 0.121703, 0.065984, 0.028002, 0.0093};
#endif

// Imagine you are sampling a 3x3 area:
// +-+-+-+
// |c|b|c|
// +-+-+-+
// |b|a|b|
// +-+-+-+
// |c|b|c|
// +-+-+-+
// It's BOX_WEIGHTS[0] for the a texels. BOX_WEIGHTS[1] for the b texels. BOX_WEIGHTS[2] for the c texels.
// Note: BOX_WEIGHTS[0] + BOX_WEIGHTS[1] * 4 + BOX_WEIGHTS[2] * 4 == 1.0
const vec3 BOX_WEIGHTS = vec3(0.25, 0.125, 0.0625);

#endif
