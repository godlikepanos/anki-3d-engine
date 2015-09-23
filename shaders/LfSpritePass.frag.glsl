// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// LF sprites frag shader

#pragma anki type frag
#pragma anki include "shaders/Common.glsl"

layout(UBO_BINDING(0, 0)) uniform sampler2DArray u_tex;

layout(location = 0) in vec3 in_uv;
layout(location = 1) flat in vec4 in_color;

layout(location = 0) out vec3 outColor;

void main()
{
	vec3 col = texture(u_tex, in_uv).rgb;

	outColor = col * in_color.rgb;
	outColor *= in_color.a;
}
