// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki type frag
#pragma anki include "shaders/Common.glsl"
#pragma anki include "shaders/Tonemapping.glsl"

// Vars
layout(binding = 0) uniform lowp sampler2D u_tex; ///< Its the IS RT

layout(std140, binding = 0) uniform _blk
{
	vec4 u_thresholdScalePad2;
};

layout(std140, binding = 0) readonly buffer _blk1
{
	vec4 u_averageLuminancePad3;
};

layout(location = 0) in vec2 in_texCoord;
layout(location = 0) out vec3 out_color;

// Consts
const uint MIPMAP = 6;

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
	out_color = readTexture(MIPMAP);
	out_color += readTexture(MIPMAP - 1);
	out_color += readTexture(MIPMAP - 2);
	
	out_color /= 3.0;
	out_color = tonemap(out_color, u_averageLuminancePad3.x, 
		u_thresholdScalePad2.x) * u_thresholdScalePad2.y;
}
