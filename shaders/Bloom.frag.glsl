// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"
#include "shaders/Tonemapping.glsl"

// Vars
layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_tex; ///< Its the IS RT

layout(ANKI_UBO_BINDING(0, 0), std140) uniform u0_
{
	vec4 u_thresholdScalePad2;
};

layout(ANKI_SS_BINDING(0, 0), std140) readonly buffer ss0_
{
	vec4 u_averageLuminancePad3;
};

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

void main()
{
	const vec2 TEXEL_SIZE = 1.0 / vec2(WIDTH, HEIGHT);

	vec3 color = textureLod(u_tex, in_uv, 0.0).rgb;
	color += textureLod(u_tex, in_uv + TEXEL_SIZE, 0.0).rgb;
	color += textureLod(u_tex, in_uv - TEXEL_SIZE, 0.0).rgb;
	color += textureLod(u_tex, in_uv + vec2(TEXEL_SIZE.x, -TEXEL_SIZE.y), 0.0).rgb;
	color += textureLod(u_tex, in_uv + vec2(-TEXEL_SIZE.x, TEXEL_SIZE.y), 0.0).rgb;

	color /= 5.0;

	color = tonemap(color, u_averageLuminancePad3.x, u_thresholdScalePad2.x) * u_thresholdScalePad2.y;

	out_color = vec4(color, 0.25);
}
