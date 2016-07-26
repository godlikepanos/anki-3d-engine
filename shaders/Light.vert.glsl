// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.h"

layout(location = 0) vec3 in_position;

layout(location = 0) vec3 out_posViewSpace;

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(ANKI_UBO_BINDING(0, 0)) uniform u0_
{
	mat4 u_mvp;
	mat4 u_mv;
};

void main()
{
	gl_Position = u_mvp * vec4(in_position, 1.0);
	out_posViewSpace = (u_mv * vec4(in_position, 1.0)).xyz;
};
