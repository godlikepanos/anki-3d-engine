// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// PPS LF sprites vert shader
#pragma anki type vert

// Per flare information
struct Flare
{
	vec4 posScale; // xy: Position, z: Scale, w: Scale again
	vec4 alphaDepth; // x: alpha, y: texture depth
};

// The block contains data for all flares
layout(std140) readonly buffer bFlares
{
	Flare uFlares[MAX_FLARES];
};

layout(location = 0) in vec2 inPosition;

layout(location = 0) out vec3 outTexCoord;
layout(location = 1) flat out float outAlpha;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	Flare flare = uFlares[gl_InstanceID];

	outTexCoord = vec3((inPosition * 0.5) + 0.5, flare.alphaDepth.y);

	vec4 posScale = flare.posScale;
	gl_Position = vec4(inPosition * posScale.zw + posScale.xy , 0.0, 1.0);

	outAlpha = flare.alphaDepth.x;
}
