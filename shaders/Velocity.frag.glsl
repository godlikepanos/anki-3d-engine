// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Functions.glsl"

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec2 out_color;

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_prevDepthRt;

layout(ANKI_UBO_BINDING(0, 0), row_major) uniform u_
{
	mat4 u_prevInvViewProjMat;
	mat4 u_crntViewProjMat;
};

void main()
{
	vec2 ndc = 2.0 * in_uv - 1.0;

	// Get prev pos in w space
	float prevDepth = texture(u_prevDepthRt, in_uv).r * 2.0 - 1.0;
	vec4 prevWPos4 = u_prevInvViewProjMat * vec4(ndc, prevDepth, 1.0);
	prevWPos4 = prevWPos4 / prevWPos4.w;

	// Project prev pos
	vec4 pos = u_crntViewProjMat * prevWPos4;
	pos /= pos.w;
	vec2 estNdc = pos.xy;

	// Write result
	/*if(gl_FragCoord.x > 1920.0/2./2.)
	{
		out_color = vec2(length(ndc));
	}
	else
	{
		out_color = vec2(length(estNdc));
	}*/

	if(pos.x < 1.0 && pos.y < 1.0 && pos.z < 1.0 && pos.x > -1.0 && pos.y > -1.0 && pos.z > -1.0)
	{
		vec2 diff = estNdc - ndc;
		out_color = (diff * 0.5 + 0.5);
	}
	else
	{
		out_color = vec2(1.0);
	}
}

