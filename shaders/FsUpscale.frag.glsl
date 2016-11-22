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
#if SSAO_ENABLED
layout(ANKI_TEX_BINDING(0, 3)) uniform sampler2D u_ssaoTex;
#endif

const float DEPTH_THRESHOLD = 1.0 / 4000.0;

layout(ANKI_UBO_BINDING(0, 0)) uniform u0_
{
	vec4 u_linearizeCfPad2;
};

void main()
{
#if 0
	// Get the depth of the current fragment
	vec3 color = nearestDepthUpscale(in_uv, u_depthFullTex, u_depthHalfTex, u_fsRt, DEPTH_THRESHOLD);
#else
	vec3 color =
		bilateralUpsample(u_depthFullTex, u_depthHalfTex, u_fsRt, 1.0 / vec2(SRC_SIZE), in_uv, u_linearizeCfPad2.xy);
#endif

#if SSAO_ENABLED
	float ssao = textureLod(u_ssaoTex, in_uv, 0.0).r;
	out_color = vec4(color, ssao);
#else
	out_color = vec4(color, 1.0);
#endif
}
