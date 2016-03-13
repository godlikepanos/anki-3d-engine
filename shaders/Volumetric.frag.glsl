// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"
#include "shaders/Functions.glsl"

layout(location = 0) in vec2 in_uv;

layout(binding = 0) uniform sampler2D u_msDepthRt;

layout(std140, UBO_BINDING(0, 0)) uniform ubo0_
{
	vec4 u_linearizePad2;
	vec4 u_fogColorFogFactor;
};

layout(location = 0) out vec3 out_color;

//==============================================================================
vec3 fog(vec2 uv)
{
	float depth = textureLod(u_msDepthRt, uv, 1.0).r;

	float linearDepth =
		linearizeDepthOptimal(depth, u_linearizePad2.x, u_linearizePad2.y);

	float t = linearDepth * u_fogColorFogFactor.w;
	return dither(u_fogColorFogFactor.rgb * t, 4.0);
}

//==============================================================================
void main()
{
	out_color = fog(in_uv);
}
