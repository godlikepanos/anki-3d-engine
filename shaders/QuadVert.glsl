// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <shaders/Common.glsl>

out gl_PerVertex
{
	Vec4 gl_Position;
};

layout(location = 0) out Vec2 out_uv;

void main()
{
	out_uv = Vec2(gl_VertexID & 1, gl_VertexID >> 1) * 2.0;
	Vec2 pos = out_uv * 2.0 - 1.0;

	gl_Position = Vec4(pos, 0.0, 1.0);
}
