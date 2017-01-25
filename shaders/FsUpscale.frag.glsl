// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"
#include "shaders/Functions.glsl"

#define BLUE_NOISE 1

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_color;

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_depthFullTex;
layout(ANKI_TEX_BINDING(0, 1)) uniform sampler2D u_depthHalfTex;
layout(ANKI_TEX_BINDING(0, 2)) uniform sampler2D u_fsRt;
#if BLUE_NOISE
layout(ANKI_TEX_BINDING(0, 3)) uniform sampler2DArray u_noiseTex;
#endif

layout(ANKI_UBO_BINDING(0, 0)) uniform u0_
{
	vec4 u_linearizeCfPad2;
};

void main()
{
	out_color =
		bilateralUpsample(u_depthFullTex, u_depthHalfTex, u_fsRt, 1.0 / vec2(SRC_SIZE), in_uv, u_linearizeCfPad2.xy);

#if BLUE_NOISE
	vec3 blueNoise = texture(u_noiseTex, vec3(FB_SIZE / vec2(NOISE_TEX_SIZE) * in_uv, 0.0), 0.0).rgb;
	blueNoise = blueNoise * 2.0 - 1.0;
	blueNoise = sign(blueNoise) * (1.0 - sqrt(1.0 - abs(blueNoise)));
	out_color.rgb += blueNoise / 32.0;
#endif
}
