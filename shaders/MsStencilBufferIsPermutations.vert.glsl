// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_mvp0;
layout(location = 2) in vec4 in_mvp1;
layout(location = 3) in vec4 in_mvp2;
layout(location = 4) in vec4 in_mvp3;

void main()
{
	mat4 mvp = mat4(in_mvp0, in_mvp1, in_mvp2, in_mvp3);
	ANKI_WRITE_POSITION(mvp * vec4(in_position, 1.0));
}
