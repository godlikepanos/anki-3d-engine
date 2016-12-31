// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"

layout(ANKI_TEX_BINDING(0, 0)) uniform lowp sampler2D uTex;

layout(location = 0) in vec2 inTexCoords;

layout(location = 0) out vec3 outColor;

void main()
{
	outColor = texture(uTex, inTexCoords).rgb;
}
