// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Functions.glsl"

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec3 out_color;

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_depthFullTex;
layout(ANKI_TEX_BINDING(0, 1)) uniform sampler2D u_depthHalfTex;
layout(ANKI_TEX_BINDING(0, 2)) uniform sampler2D u_colorTex;

const float DEPTH_THRESHOLD = 1.0 / 5000.0;

void main()
{
	out_color = nearestDepthUpscale(in_uv, u_depthFullTex, u_depthHalfTex, u_colorTex, DEPTH_THRESHOLD);
}
