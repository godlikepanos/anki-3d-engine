// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Functions.glsl"
#include "shaders/GaussianBlurCommon.glsl"

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
layout(ANKI_TEX_BINDING(0, 1)) uniform sampler2D u_depthTex;

layout(location = 0) in vec2 in_uv;
layout(location = 0) out COL_TYPE out_color;

layout(std140, ANKI_UBO_BINDING(0, 0)) uniform ubo0_
{
	vec4 u_linearizeDepthCfDepthThresholdPad1;
};

#define u_linearizeDepthCf u_linearizeDepthCfDepthThresholdPad1.xy
#define u_depthThreshold u_linearizeDepthCfDepthThresholdPad1.z

float readLinearDepth(vec2 uv)
{
	float d = texture(u_depthTex, uv).r;
	return linearizeDepthOptimal(d, u_linearizeDepthCf.x, u_linearizeDepthCf.y);
}

float computeDepthWeight(vec2 uv, float refDepth)
{
	float d = readLinearDepth(uv);
	float diff = abs(refDepth - d);
	float depthWeight = 1.0 / (EPSILON + diff);
	return depthWeight;
}

void main()
{
#if defined(VPASS)
	const vec2 TEXEL_SIZE = vec2(0.0, 1.0 / TEXTURE_SIZE.y);
#else
	const vec2 TEXEL_SIZE = vec2(1.0 / TEXTURE_SIZE.x, 0.0);
#endif

	out_color = COL_TYPE(0.0);
	float refDepth = readLinearDepth(in_uv);
	float weight = 0.0;

	for(uint i = 0u; i < STEP_COUNT; ++i)
	{
		vec2 texCoordOffset = OFFSETS[i] * TEXEL_SIZE;

		vec2 uv = in_uv + texCoordOffset;
		float w = WEIGHTS[i] * computeDepthWeight(uv, refDepth);
		out_color += texture(u_colorTex, uv).TEX_FETCH * w;
		weight += w;

		uv = in_uv - texCoordOffset;
		w = WEIGHTS[i] * computeDepthWeight(uv, refDepth);
		out_color += texture(u_colorTex, uv).TEX_FETCH * w;
		weight += w;
	}

	out_color = out_color / weight;
}
