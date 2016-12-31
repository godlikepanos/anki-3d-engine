// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"

layout(ANKI_TEX_BINDING(0, 0)) uniform mediump sampler2D u_tex;

layout(location = 0) in vec2 in_texCoord;
layout(location = 0) out vec3 out_color;

void main()
{
	const vec2 TEXEL_SIZE = 1.0 / vec2(WIDTH, HEIGHT);
	const float MIPMAP = 0.0;

	out_color = textureLod(u_tex, in_texCoord, MIPMAP).rgb;
	out_color += textureLod(u_tex, in_texCoord + TEXEL_SIZE, MIPMAP).rgb;
	out_color += textureLod(u_tex, in_texCoord - TEXEL_SIZE, MIPMAP).rgb;
	out_color += textureLod(u_tex, in_texCoord + vec2(TEXEL_SIZE.x, -TEXEL_SIZE.y), MIPMAP).rgb;
	out_color += textureLod(u_tex, in_texCoord + vec2(-TEXEL_SIZE.x, TEXEL_SIZE.y), MIPMAP).rgb;

	out_color /= 5.0;
}
