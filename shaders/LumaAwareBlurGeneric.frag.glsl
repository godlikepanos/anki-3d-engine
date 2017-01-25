// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Functions.glsl"
#include "shaders/GaussianBlurCommon.glsl"
#include "shaders/Tonemapping.glsl"

// Preprocessor switches sanity checks
#if !defined(VPASS) && !defined(HPASS)
#error See file
#endif

#if !(defined(COL_RGBA) || defined(COL_RGB) || defined(COL_R))
#error See file
#endif

#if !defined(TEXTURE_SIZE)
#error See file
#endif

// Determine color type
#if defined(COL_RGBA)
#define COL_TYPE vec4
#elif defined(COL_RGB)
#define COL_TYPE vec3
#elif defined(COL_R)
#define COL_TYPE float
#endif

// Determine tex fetch
#if defined(COL_RGBA)
#define TEX_FETCH rgba
#elif defined(COL_RGB)
#define TEX_FETCH rgb
#elif defined(COL_R)
#define TEX_FETCH r
#endif

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_colorTex;

layout(location = 0) in vec2 in_uv;
layout(location = 0) out COL_TYPE out_color;

float computeLumaWeight(float refLuma, COL_TYPE col)
{
	float l = computeLuminance(col);
	float diff = abs(refLuma - l);
	float weight = 1.0 / (EPSILON + diff);
	return weight;
}

void main()
{
#if defined(VPASS)
	const vec2 TEXEL_SIZE = vec2(0.0, 1.0 / TEXTURE_SIZE.y);
#else
	const vec2 TEXEL_SIZE = vec2(1.0 / TEXTURE_SIZE.x, 0.0);
#endif

	out_color = COL_TYPE(0.0);
	float refLuma = computeLuminance(texture(u_colorTex, in_uv).TEX_FETCH);
	float weight = 0.0;

	for(uint i = 0u; i < STEP_COUNT; ++i)
	{
		vec2 texCoordOffset = OFFSETS[i] * TEXEL_SIZE;

		vec2 uv = in_uv + texCoordOffset;
		COL_TYPE col = texture(u_colorTex, uv).TEX_FETCH;
		float w = WEIGHTS[i] * computeLumaWeight(refLuma, col);
		out_color += col * w;
		weight += w;

		uv = in_uv - texCoordOffset;
		col = texture(u_colorTex, uv).TEX_FETCH;
		w = WEIGHTS[i] * computeLumaWeight(refLuma, col);
		out_color += col * w;
		weight += w;
	}

	out_color = out_color / weight;
}
