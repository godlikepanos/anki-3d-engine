// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"

#if !defined(UV_OFFSET)
#define UV_OFFSET (0.0)
#endif

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) out vec2 out_uv;

void main()
{
	const vec2 POSITIONS[3] = vec2[](vec2(-1.0, -1.0), vec2(3.0, -1.0), vec2(-1.0, 3.0));

	vec2 pos = POSITIONS[gl_VertexID];
	out_uv = pos * 0.5 + (0.5 + UV_OFFSET);
	ANKI_WRITE_POSITION(vec4(pos, 0.0, 1.0));
}
