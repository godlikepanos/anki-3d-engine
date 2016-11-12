// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"
#include "shaders/Functions.glsl"

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_color;

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_depthFullTex;
layout(ANKI_TEX_BINDING(0, 1)) uniform sampler2D u_depthHalfTex;
layout(ANKI_TEX_BINDING(0, 2)) uniform sampler2D u_colorTexNearest;
layout(ANKI_TEX_BINDING(0, 3)) uniform sampler2D u_colorTexLinear;
layout(ANKI_TEX_BINDING(0, 4)) uniform sampler2D u_volTex;
#if SSAO_ENABLED
layout(ANKI_TEX_BINDING(0, 5)) uniform sampler2D u_ssaoTex;
#endif

layout(ANKI_UBO_BINDING(0, 0)) uniform _u0
{
	vec4 u_linearizePad2;
};

const float DEPTH_THRESHOLD = 1.0 / 1000.0;

const vec2 COLOR_TEX_TEXEL_SIZE = 1.0 / vec2(TEXTURE_WIDTH, TEXTURE_HEIGHT);

const vec2 OFFSETS[8] = vec2[](vec2(-COLOR_TEX_TEXEL_SIZE.x, -COLOR_TEX_TEXEL_SIZE.y),
	vec2(0.0, -COLOR_TEX_TEXEL_SIZE.y),
	vec2(COLOR_TEX_TEXEL_SIZE.x, -COLOR_TEX_TEXEL_SIZE.y),
	vec2(COLOR_TEX_TEXEL_SIZE.x, 0.0),
	vec2(COLOR_TEX_TEXEL_SIZE.x, COLOR_TEX_TEXEL_SIZE.y),
	vec2(0.0, COLOR_TEX_TEXEL_SIZE.y),
	vec2(-COLOR_TEX_TEXEL_SIZE.x, COLOR_TEX_TEXEL_SIZE.y),
	vec2(-COLOR_TEX_TEXEL_SIZE.x, 0.0));

void main()
{
	// Get the depth of the current fragment
	float depth = texture(u_depthFullTex, in_uv).r;

	// Gather the depths around the current fragment and:
	// - Get the min difference compared to crnt depth
	// - Get the new UV that is closer to the crnt depth
	float lowDepth = texture(u_depthHalfTex, in_uv).r;
	float minDiff = abs(depth - lowDepth);
	float maxDiff = minDiff;
	vec2 newUv = in_uv;
	for(uint i = 0u; i < 8u; ++i)
	{
		// Read the low depth
		vec2 uv = in_uv + OFFSETS[i];
		float lowDepth = texture(u_depthHalfTex, uv).r;

		// Update the max difference compared to the current fragment
		float diff = abs(depth - lowDepth);
		maxDiff = max(maxDiff, diff);

		// Update the new UV
		if(diff < minDiff)
		{
			minDiff = diff;
			newUv = uv;
		}
	}

// Check for depth discontinuites. Use textureLod because it's undefined if you are sampling mipmaped in a conditional
// branch. See http://teknicool.tumblr.com/post/77263472964/glsl-dynamic-branching-and-texture-samplers
#if 0
	float a = u_linearizePad2.x;
	float b = u_linearizePad2.y;
	float maxDiffLinear = abs(linearizeDepthOptimal(depth + maxDiff, a, b)
		- linearizeDepthOptimal(depth, a, b));
#else
	float maxDiffLinear = abs(maxDiff);
#endif
	vec3 color = textureLod(u_volTex, in_uv, 0.0).rgb;
	if(maxDiffLinear < DEPTH_THRESHOLD)
	{
		// No major discontinuites, sample with bilinear
		color += textureLod(u_colorTexLinear, in_uv, 0.0).rgb;
	}
	else
	{
		// Some discontinuites, need to use the newUv
		color += textureLod(u_colorTexNearest, newUv, 0.0).rgb;
	}

#if SSAO_ENABLED
	float ssao = texture(u_ssaoTex, in_uv).r;
	out_color = vec4(color, ssao);
#else
	out_color = vec4(color, 1.0);
#endif
}
