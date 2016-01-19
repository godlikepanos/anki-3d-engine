// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) out vec2 out_texCoord;

void main()
{
	const vec2 POSITIONS[3] =
		vec2[](vec2(-1.0, -1.0), vec2(3.0, -1.0), vec2(-1.0, 3.0));

	vec2 pos = POSITIONS[gl_VertexID];
	out_texCoord = pos * 0.5 + 0.5;
	gl_Position = vec4(pos, 0.0, 1.0);
}
