// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// This file contains common code for all shaders. It's optional but it's recomended to include it

#pragma once

#include <anki/shaders/TextureFunctions.glsl>

// WORKAROUNDS
#if defined(ANKI_VENDOR_NVIDIA)
#	define NVIDIA_LINK_ERROR_WORKAROUND 1
#else
#	define NVIDIA_LINK_ERROR_WORKAROUND 0
#endif

#if defined(ANKI_VENDOR_AMD) && defined(ANKI_BACKEND_VULKAN)
#	define AMD_VK_READ_FIRST_INVOCATION_COMPILER_CRASH 1
#else
#	define AMD_VK_READ_FIRST_INVOCATION_COMPILER_CRASH 0
#endif

// Default precision
#ifndef DEFAULT_FLOAT_PRECISION
#	define DEFAULT_FLOAT_PRECISION highp
#endif

#ifndef DEFAULT_INT_PRECISION
#	define DEFAULT_INT_PRECISION highp
#endif

// Constants
precision DEFAULT_FLOAT_PRECISION F32;
precision DEFAULT_INT_PRECISION I32;

const F32 EPSILON = 0.000001;
const F32 FLT_MAX = 3.402823e+38;
const U32 MAX_U32 = 0xFFFFFFFFu;

const F32 PI = 3.14159265358979323846;
const U32 UBO_MAX_SIZE = 16384u;
const U32 MAX_SHARED_MEMORY = 32u * 1024u;

// Macros
#define UV_TO_NDC(x_) ((x_)*2.0 - 1.0)
#define NDC_TO_UV(x_) ((x_)*0.5 + 0.5)
#define saturate(x_) clamp((x_), 0.0, 1.0)
#define mad(a_, b_, c_) fma((a_), (b_), (c_))

// Common locations
#define POSITION_LOCATION 0
#define TEXTURE_COORDINATE_LOCATION 1
#define TEXTURE_COORDINATE_LOCATION_2 2
#define NORMAL_LOCATION 3
#define TANGENT_LOCATION 4
#define COLOR_LOCATION 5
#define BONE_WEIGHTS_LOCATION 6
#define BONE_INDICES_LOCATION 7

#define SCALE_LOCATION 1
#define ALPHA_LOCATION 2

// Passes
#define PASS_GB 0
#define PASS_FS 1
#define PASS_SM 2
#define PASS_EZ 3

// Other
#if defined(ANKI_BACKEND_VULKAN) && ANKI_BACKEND_MAJOR >= 1 && ANKI_BACKEND_MINOR >= 1
#	define UNIFORM(x_) subgroupBroadcastFirst(x_)
#else
#	define UNIFORM(x_) x_
#endif

#define CALC_BITANGENT_IN_VERT 1
