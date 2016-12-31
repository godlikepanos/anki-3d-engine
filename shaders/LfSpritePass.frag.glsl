// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// LF sprites frag shader

#include "shaders/Common.glsl"

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2DArray u_tex;

layout(location = 0) in vec3 in_uv;
layout(location = 1) flat in vec4 in_color;

layout(location = 0) out vec4 out_color;

void main()
{
	vec4 col = texture(u_tex, in_uv);
	out_color = col * in_color;
}
