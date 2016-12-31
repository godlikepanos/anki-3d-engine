// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"

#define SMAA_GLSL_4
#define SMAA_INCLUDE_PS 0
#define SMAA_INCLUDE_VS 1
#include "shaders/SMAA.hlsl"

layout(location = 0) out vec2 out_uv;
layout(location = 1) out vec2 out_pixcoord;
layout(location = 2) out vec4 out_offset0;
layout(location = 3) out vec4 out_offset1;
layout(location = 4) out vec4 out_offset2;

void main(void)
{
	const vec2 POSITIONS[3] = vec2[](vec2(-1.0, -1.0), vec2(3.0, -1.0), vec2(-1.0, 3.0));
	vec2 pos = POSITIONS[gl_VertexID];
	out_uv = pos * 0.5 + 0.5;

	vec4 offsets[3];
	offsets[0] = vec4(0.0);
	offsets[1] = vec4(0.0);
	offsets[2] = vec4(0.0);
	out_pixcoord = vec2(0.0);
	SMAABlendingWeightCalculationVS(out_uv, out_pixcoord, offsets);
	out_offset0 = offsets[0];
	out_offset1 = offsets[1];
	out_offset2 = offsets[2];

	ANKI_WRITE_POSITION(vec4(pos, 0.0, 1.0));
}
