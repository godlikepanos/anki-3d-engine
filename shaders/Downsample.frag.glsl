// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_rt0;
layout(ANKI_TEX_BINDING(0, 1)) uniform sampler2D u_rt1;
layout(ANKI_TEX_BINDING(0, 2)) uniform sampler2D u_rt2;
layout(ANKI_TEX_BINDING(0, 3)) uniform sampler2D u_depth;
layout(ANKI_TEX_BINDING(0, 4)) uniform sampler2D u_isRt;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_rt0;
layout(location = 1) out vec4 out_rt1;
layout(location = 2) out vec4 out_rt2;
layout(location = 3) out vec3 out_isRt;

void main()
{
	out_rt0 = texture(u_rt0, in_uv);
	out_rt1 = texture(u_rt1, in_uv);
	out_rt2 = texture(u_rt2, in_uv);
	gl_FragDepth = texture(u_depth, in_uv).r;
	out_isRt = texture(u_isRt, in_uv).rgb;
}
