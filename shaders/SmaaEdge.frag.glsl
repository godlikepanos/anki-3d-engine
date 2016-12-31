// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"
#define SMAA_GLSL_4
#define SMAA_INCLUDE_PS 1
#define SMAA_INCLUDE_VS 0
#include "shaders/SMAA.hlsl"

layout(location = 0) out vec2 out_color;

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_isRt;

layout(location = 0) in vec2 in_uv;
layout(location = 1) in vec4 in_offset0;
layout(location = 2) in vec4 in_offset1;
layout(location = 3) in vec4 in_offset2;

void main()
{
	vec4 offsets[3];
	offsets[0] = in_offset0;
	offsets[1] = in_offset1;
	offsets[2] = in_offset2;
	out_color = SMAAColorEdgeDetectionPS(in_uv, offsets, u_isRt);
}
