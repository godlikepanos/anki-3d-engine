// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
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

layout(location = 0) in vec2 in_texCoord;
layout(location = 0) out vec3 out_color;

void main()
{
	const vec2 TEXEL_SIZE = 1.0 / vec2(WIDTH, HEIGHT);

	out_color = textureLod(u_tex, in_texCoord, MIPMAP).rgb;
	out_color += textureLod(u_tex, in_texCoord + TEXEL_SIZE, MIPMAP).rgb;
	out_color += textureLod(u_tex, in_texCoord - TEXEL_SIZE, MIPMAP).rgb;
	out_color += textureLod(u_tex, in_texCoord + vec2(TEXEL_SIZE.x, -TEXEL_SIZE.y), MIPMAP).rgb;
	out_color += textureLod(u_tex, in_texCoord + vec2(-TEXEL_SIZE.x, TEXEL_SIZE.y), MIPMAP).rgb;

	out_color /= 5.0;

	out_color = tonemap(out_color, u_averageLuminancePad3.x, u_thresholdScalePad2.x) * u_thresholdScalePad2.y;
}
