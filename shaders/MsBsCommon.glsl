// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki include "shaders/Common.glsl"

// Generic functions because materials cannot use operators
#define add_DEFINED
#define add(a, b) ((a) + (b))

#define mul_DEFINED
#define mul(a, b) ((a) * (b))

#define assign_DEFINED
#define assign(a, b) ((a) = (b))

#define vec4ToVec3_DEFINED
#define vec4ToVec3(a) ((a).xyz)

#define vec3ToVec4_DEFINED
#define vec3ToVec4(a, w) (vec4((a), (w)))

#define getW_DEFINED
#define getW(a) ((a).w)

#define setW_DEFINED
#define setW(a, b) ((a).w = (b))

// Misc
#define COLOR 0
#define DEPTH 1
