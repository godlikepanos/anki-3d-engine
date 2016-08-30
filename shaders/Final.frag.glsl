// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
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
	vec3 col = textureLod(u_tex, in_texCoord, 0.0).rgb;
	out_color = col;
}
