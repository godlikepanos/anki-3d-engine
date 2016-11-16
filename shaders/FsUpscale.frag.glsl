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
layout(ANKI_TEX_BINDING(0, 2)) uniform sampler2D u_fsRt;
layout(ANKI_TEX_BINDING(0, 3)) uniform sampler2D u_volTex;
#if SSAO_ENABLED
layout(ANKI_TEX_BINDING(0, 4)) uniform sampler2D u_ssaoTex;
#endif

layout(ANKI_UBO_BINDING(0, 0)) uniform _u0
{
	vec4 u_linearizePad2;
};

const float DEPTH_THRESHOLD = 1.0 / 1000.0;

const vec2 COLOR_TEX_TEXEL_SIZE = 1.0 / vec2(TEXTURE_WIDTH, TEXTURE_HEIGHT);

void main()
{
	// Get the depth of the current fragment
	float depth = texture(u_depthFullTex, in_uv).r;
	vec3 color = textureLod(u_volTex, in_uv, 0.0).rgb;

	vec4 lowDepths = textureGather(u_depthHalfTex, in_uv, 0);
	vec4 diffs = abs(vec4(depth) - lowDepths);

	if(all(lessThan(diffs, vec4(DEPTH_THRESHOLD))))
	{
		// No major discontinuites, sample with bilinear
		color += textureLod(u_fsRt, in_uv, 0.0).rgb;
	}
	else
	{
		// Some discontinuites, need to use the newUv
		vec4 r = textureGather(u_fsRt, in_uv, 0);
		vec4 g = textureGather(u_fsRt, in_uv, 1);
		vec4 b = textureGather(u_fsRt, in_uv, 2);

		float minDiff = diffs.x;
		uint comp = 0;

		if(diffs.y < minDiff)
		{
			comp = 1;
			minDiff = diffs.y;
		}

		if(diffs.z < minDiff)
		{
			comp = 2;
			minDiff = diffs.z;
		}

		if(diffs.w < minDiff)
		{
			comp = 3;
		}

		color += vec3(r[comp], g[comp], b[comp]);
	}

#if SSAO_ENABLED
	float ssao = textureLod(u_ssaoTex, in_uv, 0.0).r;
	out_color = vec4(color, ssao);
#else
	out_color = vec4(color, 1.0);
#endif
}
