// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Defines it needs:
// HORIZONTAL | VERTICAL | BOX
// COLOR_COMPONENTS
// WORKGROUP_SIZE (only for compute)
// TEXTURE_SIZE
// SAMPLE_COUNT (must be odd number)

#ifndef ANKI_SHADERS_DEPTH_AWARE_BLUR_GLSL
#define ANKI_SHADERS_DEPTH_AWARE_BLUR_GLSL

#include "shaders/Common.glsl"

#if SAMPLE_COUNT < 3
#	error See file
#endif

#if defined(ANKI_COMPUTE_SHADER)
#	define USE_COMPUTE 1
#else
#	define USE_COMPUTE 0
#endif

// Define some macros depending on the number of components
#if COLOR_COMPONENTS == 4
#	define COL_TYPE vec4
#	define TEX_FETCH rgba
#	define TO_VEC4(x_) x_
#elif COLOR_COMPONENTS == 3
#	define COL_TYPE vec3
#	define TEX_FETCH rgb
#	define TO_VEC4(x_) vec4(x_, 0.0)
#elif COLOR_COMPONENTS == 1
#	define COL_TYPE float
#	define TEX_FETCH r
#	define TO_VEC4(x_) vec4(x_, 0.0, 0.0, 0.0)
#else
#	error See file
#endif

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_inTex;
layout(ANKI_TEX_BINDING(0, 1)) uniform sampler2D u_depthTex;

#if USE_COMPUTE
layout(local_size_x = WORKGROUP_SIZE.x, local_size_y = WORKGROUP_SIZE.y, local_size_z = 1) in;
layout(ANKI_IMAGE_BINDING(0, 0)) writeonly uniform image2D u_outImg;
#else
layout(location = 0) in vec2 in_uv;
layout(location = 0) out COL_TYPE out_color;
#endif

float computeDepthWeight(float refDepth, float depth)
{
	float diff = abs(refDepth - depth);
	float weight = 1.0 / (EPSILON + diff);
	return sqrt(weight);
}

float readDepth(vec2 uv)
{
	return textureLod(u_depthTex, uv, 0.0).r;
}

void sampleTex(vec2 uv, float refDepth, inout COL_TYPE col, inout float weight)
{
	COL_TYPE color = textureLod(u_inTex, uv, 0.0).TEX_FETCH;
	float w = computeDepthWeight(refDepth, readDepth(uv));
	col += color * w;
	weight += w;
}

void main()
{
	// Compute texel size
#if defined(HORIZONTAL)
	const vec2 TEXEL_SIZE = vec2(1.0 / float(TEXTURE_SIZE.x), 0.0);
#elif defined(VERTICAL)
	const vec2 TEXEL_SIZE = vec2(0.0, 1.0 / float(TEXTURE_SIZE.y));
#endif

	// Set UVs
#if USE_COMPUTE
	ANKI_BRANCH if(gl_GlobalInvocationID.x >= TEXTURE_SIZE.x || gl_GlobalInvocationID.y >= TEXTURE_SIZE.y)
	{
		// Out of bounds
		return;
	}

	vec2 uv = (vec2(gl_GlobalInvocationID.xy) + 0.5) / vec2(TEXTURE_SIZE);
#else
	vec2 uv = in_uv;
#endif

	// Sample
	COL_TYPE color = textureLod(u_inTex, uv, 0.0).TEX_FETCH;
	float refDepth = readDepth(uv);
	float weight = 1.0;

#if !defined(BOX)
	// Do seperable

	vec2 texCoordOffset = 1.5 * TEXEL_SIZE;

	ANKI_UNROLL for(uint i = 0u; i < (SAMPLE_COUNT - 1u) / 2u; ++i)
	{
		sampleTex(uv + texCoordOffset, refDepth, color, weight);
		sampleTex(uv - texCoordOffset, refDepth, color, weight);

		texCoordOffset += 2.0 * TEXEL_SIZE;
	}
#else
	// Do box

	const vec2 OFFSET = 1.5 * (1.0 / vec2(TEXTURE_SIZE));

	sampleTex(uv + vec2(+OFFSET.x, +OFFSET.y), refDepth, color, weight);
	sampleTex(uv + vec2(+OFFSET.x, -OFFSET.y), refDepth, color, weight);
	sampleTex(uv + vec2(-OFFSET.x, +OFFSET.y), refDepth, color, weight);
	sampleTex(uv + vec2(-OFFSET.x, -OFFSET.y), refDepth, color, weight);

	sampleTex(uv + vec2(OFFSET.x, 0.0), refDepth, color, weight);
	sampleTex(uv + vec2(0.0, OFFSET.y), refDepth, color, weight);
	sampleTex(uv + vec2(-OFFSET.x, 0.0), refDepth, color, weight);
	sampleTex(uv + vec2(0.0, -OFFSET.y), refDepth, color, weight);
#endif
	color = color / weight;

	// Write value
#if USE_COMPUTE
	imageStore(u_outImg, ivec2(gl_GlobalInvocationID.xy), TO_VEC4(color));
#else
	out_color = color;
#endif
}

#endif
