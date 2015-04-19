// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki type frag
#pragma anki include "shaders/Common.glsl"

// Vars
layout(binding = 0) uniform lowp sampler2D u_tex; ///< Its the IS RT

layout(std140) uniform _blk
{
	vec4 u_exposureComp;
};

layout(location = 0) in vec2 in_texCoord;
layout(location = 0) out vec3 out_color;

// Consts
const uint MIPMAP = 4;
const uint WIDTH = ANKI_RENDERER_WIDTH / (2 << (MIPMAP - 1));
const uint HEIGHT = ANKI_RENDERER_HEIGHT / (2 << (MIPMAP - 1));

const vec2 TEXTURE_SIZE = vec2(WIDTH, HEIGHT);
const vec2 SCALE = (TEXTURE_SIZE - 1.0) / TEXTURE_SIZE;
const vec2 OFFSET = 1.0 / (2.0 * TEXTURE_SIZE);

const float BRIGHT_MAX = 5.0;

void main()
{
	vec3 color = 
		textureLod(u_tex, in_texCoord + OFFSET, float(MIPMAP)).rgb;

#if 1
	float luminance = dot(vec3(0.30, 0.59, 0.11), color);
	float yd = u_exposureComp.x * (u_exposureComp.x / BRIGHT_MAX + 1.0) /
		(u_exposureComp.x + 1.0) * luminance;
	color *= yd;
#endif

	out_color = color;
}
