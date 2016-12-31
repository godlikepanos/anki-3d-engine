// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) in vec2 in_position;
layout(location = 2) in mediump vec4 in_color;

layout(location = 0) out mediump vec4 out_color;

void main()
{
	gl_Position = vec4(in_position, 0.0, 1.0);
	out_color = in_color;
}
