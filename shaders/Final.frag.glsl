// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// The final pass

#include "shaders/Common.glsl"
#include "shaders/Pack.glsl"

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_tex;

layout(location = 0) in highp vec2 in_texCoord;
layout(location = 0) out vec3 out_color;

void main()
{
	vec2 uv = in_texCoord;
#ifdef ANKI_VK
	uv.y = 1.0 - uv.y;
#endif
	vec3 col = textureLod(u_tex, uv, 0.0).rgb;
	out_color = col;
}
