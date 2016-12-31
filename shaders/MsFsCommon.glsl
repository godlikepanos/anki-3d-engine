// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_MS_FS_COMMON_GLSL
#define ANKI_SHADERS_MS_FS_COMMON_GLSL

#include "shaders/Common.glsl"

// Misc
#define COLOR 0
#define DEPTH 1

#define CALC_BITANGENT_IN_VERT 1

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

// Read from animated texture
#define readAnimatedTextureRgba_DEFINED
vec4 readAnimatedTextureRgba(sampler2DArray tex, float layerCount, float period, vec2 uv, float time)
{
	float layer = mod(time * layerCount / period, layerCount);
	return texture(tex, vec3(uv, layer));
}

#endif
