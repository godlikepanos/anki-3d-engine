// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Shaders/Common.hlsl>

#if !defined(CUSTOM_DEPTH)
#	define CUSTOM_DEPTH 0.0
#endif

struct VertOut
{
	Vec4 m_svPosition : SV_POSITION;
	Vec2 m_uv : TEXCOORD;
};

#if ANKI_VERTEX_SHADER
VertOut main(U32 vertId : SV_VERTEXID)
{
	const Vec2 coord = Vec2(vertId >> 1, vertId & 1);

	VertOut output;
	output.m_svPosition = Vec4(coord * Vec2(4.0, -4.0) + Vec2(-1.0, 1.0), CUSTOM_DEPTH, 1.0);
	output.m_uv = coord * 2.0f;

	return output;
}
#endif
