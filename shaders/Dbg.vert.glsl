// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki type vert
#pragma anki include "shaders/Common.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in mat4 inMvp;

layout(location = 0) out vec3 outColor;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	outColor = inColor;
	gl_Position = inMvp * vec4(inPosition, 1.0);
}
