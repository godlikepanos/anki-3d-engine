// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki type frag

#define DEFAULT_FLOAT_PRECISION mediump
#pragma anki include "shaders/Common.glsl"

layout(binding = 0) uniform lowp sampler2D tex; ///< Its the IS RT

layout(std140) readonly buffer commonBlock
{
	vec4 inExposureComp;
};
#define inExposure inExposureComp.x

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec3 outColor;

const float brightMax = 4.0;

void main()
{
	vec3 color = textureRt(tex, inTexCoord).rgb;

	float luminance = dot(vec3(0.30, 0.59, 0.11), color);
	float yd = inExposure * (inExposure / brightMax + 1.0) /
		(inExposure + 1.0) * luminance;
	color *= yd;
	outColor = color;
}
