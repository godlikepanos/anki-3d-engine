// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) out vec2 out_uvLow;
layout(location = 1) out vec2 out_uvHigh;

void main()
{
	const vec2 POSITIONS[3] =
		vec2[](vec2(-1.0, -1.0), vec2(3.0, -1.0), vec2(-1.0, 3.0));

	vec2 pos = POSITIONS[gl_VertexID];
	gl_Position = vec4(pos, 0.0, 1.0);

	// Compute some offset in order to align the texture coordinates to texel
	// center
	const vec2 TEXEL_SIZE_LOW =
		vec2(1.0 / float(TEXTURE_WIDTH), 1.0 / float(TEXTURE_HEIGHT));
	const vec2 UV_OFFSET_LOW = TEXEL_SIZE_LOW / 2.0;
	out_uvLow = pos * 0.5 + (0.5 + UV_OFFSET_LOW);

	const vec2 TEXEL_SIZE_HIGH =
		vec2(1.0 / float(2 * TEXTURE_WIDTH), 1.0 / float(2 * TEXTURE_HEIGHT));
	const vec2 UV_OFFSET_HIGH = TEXEL_SIZE_HIGH / 2.0;
	out_uvHigh = pos * 0.5 + (0.5 + UV_OFFSET_HIGH);
}
