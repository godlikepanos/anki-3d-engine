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
	vec4 u_exposurePad3;
};
#define u_exposure u_exposurePad3.x

layout(location = 0) in vec2 in_texCoord;
layout(location = 0) out vec3 out_color;

// Consts
const uint MIPMAP = 6;

const float BRIGHT_MAX = 5.0;

vec3 readTexture(in uint mipmap)
{
	uint w = ANKI_RENDERER_WIDTH / (2 << (mipmap - 1));
	uint h = ANKI_RENDERER_HEIGHT / (2 << (mipmap - 1));

	vec2 textureSize = vec2(w, h);
	vec2 scale = (textureSize - 1.0) / textureSize;
	vec2 offset = 1.0 / (2.0 * textureSize);

	vec2 texc = in_texCoord * scale + offset;

	return textureLod(u_tex, texc, float(mipmap)).rgb;
}

void main()
{
	vec3 color = vec3(0.0);
	
	color += readTexture(MIPMAP);
	color += readTexture(MIPMAP - 1);
	color += readTexture(MIPMAP - 2);
	
	color /= 3.0;

#if 1
	float luminance = dot(vec3(0.30, 0.59, 0.11), color);
	float yd = u_exposure * (u_exposure / BRIGHT_MAX + 1.0) /
		(u_exposure + 1.0) * luminance;
	color *= yd;
#endif

	out_color = color;
}
