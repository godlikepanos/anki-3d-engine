// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// LF occlusion vert shader

#include "shaders/Common.glsl"

layout(UBO_BINDING(0, 0), std140, row_major) uniform _block
{
	mat4 u_mvp;
};

layout(location = 0) in vec3 in_position;

out gl_PerVertex
{
	vec4 gl_Position;
	float gl_PointSize;
};

void main()
{
	gl_Position = u_mvp * vec4(in_position, 1.0);
	gl_PointSize = 16.0;
}
