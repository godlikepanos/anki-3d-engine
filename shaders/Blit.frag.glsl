// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_tex;

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec3 out_col;

void main()
{
	out_col = textureLod(u_tex, in_uv, 0.0).rgb;
}
