// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki type frag
#pragma anki include "shaders/Common.glsl"

layout(binding = 0) uniform lowp sampler2D uTex;

layout(location = 0) in F32Vec2 inTexCoords;

layout(location = 0) out F16Vec3 outColor;

void main()
{
	outColor = textureRt(uTex, inTexCoords).rgb;
}

