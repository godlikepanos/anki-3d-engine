// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"

#if SMAA_ENABLED
#define SMAA_GLSL_4
#define SMAA_INCLUDE_PS 0
#define SMAA_INCLUDE_VS 1
#include "shaders/SMAA.hlsl"
#endif

layout(location = 0) out vec2 out_uv;
#if SMAA_ENABLED
layout(location = 1) out vec4 out_smaaOffset;
#endif

void main(void)
{
	const vec2 POSITIONS[3] = vec2[](vec2(-1.0, -1.0), vec2(3.0, -1.0), vec2(-1.0, 3.0));
	vec2 pos = POSITIONS[gl_VertexID];
	out_uv = pos * 0.5 + 0.5;

	ANKI_WRITE_POSITION(vec4(pos, 0.0, 1.0));

#if SMAA_ENABLED
	out_smaaOffset = vec4(0.0);
	SMAANeighborhoodBlendingVS(out_uv, out_smaaOffset);
#endif
}
