// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// This file contains common code for all shaders. It's optional but it's recomended to include it

#ifndef ANKI_SHADERS_COMMON_GLSL
#define ANKI_SHADERS_COMMON_GLSL

// WORKAROUND
#if defined(ANKI_VENDOR_NVIDIA)
#	define NVIDIA_LINK_ERROR_WORKAROUND 1
#else
#	define NVIDIA_LINK_ERROR_WORKAROUND 0
#endif

// Default precision
#ifndef DEFAULT_FLOAT_PRECISION
#	define DEFAULT_FLOAT_PRECISION highp
#endif

#ifndef DEFAULT_INT_PRECISION
#	define DEFAULT_INT_PRECISION highp
#endif

// Constants
precision DEFAULT_FLOAT_PRECISION float;
precision DEFAULT_INT_PRECISION int;

const float EPSILON = 0.000001;
const float FLT_MAX = 3.402823e+38;
const uint MAX_U32 = 0xFFFFFFFFu;

const float PI = 3.14159265358979323846;
const uint UBO_MAX_SIZE = 16384u;

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
#define PASS_GB_FS 0
#define PASS_SM 1
#define PASS_EZ 2

// Other
#if !defined(ANKI_ARB_SHADER_BALLOT)
#	define readFirstInvocationARB(x_) (x_)
#endif

#define CALC_BITANGENT_IN_VERT 1

#endif
