// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// PPS LF sprites frag shader

#pragma anki type frag
#define DEFAULT_FLOAT_PRECISION mediump
#pragma anki include "shaders/Common.glsl"

#define SINGLE_FLARE 0

#if SINGLE_FLARE
uniform sampler2D uImage;
#else
uniform DEFAULT_FLOAT_PRECISION sampler2DArray uImages;
#endif

layout(location = 0) in vec3 inTexCoords;
layout(location = 1) flat in float inAlpha;

layout(location = 0) out vec3 outColor;

void main()
{
#if SINGLE_FLARE
	vec3 col = texture(uImage, inTexCoords.st).rgb;
#else
	vec3 col = texture(uImages, inTexCoords).rgb;
#endif

	outColor = col * inAlpha;
}
