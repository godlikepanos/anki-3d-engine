// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// This file contains common code for all shaders. It's optional but it's recomended to include it

#pragma once

#include <AnKi/Shaders/Include/Common.h>
#include <AnKi/Shaders/TextureFunctions.glsl>

// Constants
const F32 EPSILON = 0.000001;
const F32 FLT_MAX = 3.402823e+38;
const U32 MAX_U32 = 0xFFFFFFFFu;

const F32 PI = 3.14159265358979323846;
const U32 MAX_UBO_SIZE = 16384u;
const U32 MAX_SHARED_MEMORY = 32u * 1024u;

// Macros
#define UV_TO_NDC(x_) ((x_)*2.0 - 1.0)
#define NDC_TO_UV(x_) ((x_)*0.5 + 0.5)
#define saturate(x_) clamp((x_), 0.0, 1.0)
#define mad(a_, b_, c_) fma((a_), (b_), (c_))

// Passes
#define PASS_GB 0
#define PASS_FS 1
#define PASS_SM 2
#define PASS_EZ 3
