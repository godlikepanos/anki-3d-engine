// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki type vert
#pragma anki include "shaders/Common.glsl"

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) out vec2 out_uv;

void main()
{
	const vec2 POSITIONS[3] = vec2[](
		vec2(-1.0, -1.0),
		vec2(3.0, -1.0),
		vec2(-1.0, 3.0));

	vec2 pos = POSITIONS[gl_VertexID];
	gl_Position = vec4(pos, 0.0, 1.0);

	// Add to the UV and place it at the middle of the texel and slightly
	// top-right
	// +----+
	// |   o|
	// |    |
	// |    |
	// +----+
	const vec2 TEXEL_SIZE = vec2(1.0 / float(TEXTURE_WIDTH),
		1.0 / float(TEXTURE_HEIGHT));
	const vec2 UV_OFFSET = TEXEL_SIZE / 2.0 + EPSILON * 2.0;
	out_uv = pos * 0.5 + (0.5 + UV_OFFSET);
}

