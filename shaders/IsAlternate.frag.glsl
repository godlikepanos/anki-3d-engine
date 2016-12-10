// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Functions.glsl"

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec3 out_color;

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_depthRt;
layout(ANKI_TEX_BINDING(0, 1)) uniform sampler2D u_oldIsRt;
layout(ANKI_TEX_BINDING(0, 2)) uniform sampler2D u_prevDepthRt;

layout(ANKI_UBO_BINDING(0, 0), row_major) uniform u_
{
	mat4 u_invViewProjMat;
	mat4 u_prevViewProjMat;
};

void main()
{
	vec2 ndc = 2.0 * in_uv - 1.0;

	// Get crnt pos in world space
	float depth = texture(u_depthRt, in_uv).r;
	vec4 worldPos4 = u_invViewProjMat * vec4(ndc, depth * 2.0 - 1.0, 1.0);
	worldPos4 = worldPos4 / worldPos4.w;

	// Project to get old ndc
	vec4 oldNdc4 = u_prevViewProjMat * worldPos4;
	vec2 oldNdc = oldNdc4.xy / oldNdc4.w;
	
	if(oldNdc.x < 1.0 && oldNdc.y < 1.0 && oldNdc.x > -1.0 && oldNdc.y > -1.0)
	{
		vec2 oldUv = oldNdc * 0.5 + 0.5;

		// Get prev depth
		float prevDepth = textureLod(u_prevDepthRt, oldUv, 0.0).r;

		if(abs(prevDepth - depth) < 0.01)
		{
			out_color = textureLod(u_oldIsRt, oldUv, 0.0).rgb;
		}
		else
		{
			discard;
			//out_color = vec3(1.0);
		}
	}
	else
	{
		discard;
		//out_color = vec3(1.0);
	}
}
