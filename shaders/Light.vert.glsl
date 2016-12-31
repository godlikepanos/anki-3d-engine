// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"

layout(location = 0) in vec3 in_position;

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(ANKI_UBO_BINDING(0, 0), row_major) uniform u0_
{
	mat4 u_mvp;
};

void main()
{
	ANKI_WRITE_POSITION(u_mvp * vec4(in_position, 1.0));
}
