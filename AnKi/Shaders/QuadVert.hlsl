// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Shaders/Common.hlsl>

struct VertOut
{
	Vec4 m_position : SV_POSITION;
	[[vk::location(0)]] Vec2 m_uv : TEXCOORD;
};

VertOut main(U32 vertId : SV_VERTEXID)
{
	VertOut output;
	output.m_uv = Vec2(vertId & 1, vertId >> 1) * 2.0;

	output.m_position = Vec4(output.m_uv * 2.0 - 1.0, 0.0, 1.0);

	return output;
}
