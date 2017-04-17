// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_MS_FS_COMMON_GLSL
#define ANKI_SHADERS_MS_FS_COMMON_GLSL

#include "shaders/Common.glsl"

#define CALC_BITANGENT_IN_VERT 1

// Read from animated texture
#define readAnimatedTextureRgba_DEFINED
vec4 readAnimatedTextureRgba(sampler2DArray tex, float layerCount, float period, vec2 uv, float time)
{
	float layer = mod(time * layerCount / period, layerCount);
	return texture(tex, vec3(uv, layer));
}

#endif
