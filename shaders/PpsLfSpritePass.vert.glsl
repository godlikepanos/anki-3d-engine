// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
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
layout(std140) uniform _flaresBlock
{
	Flare u_flares[MAX_SPRITES];
};

layout(location = 0) in vec2 in_position;

layout(location = 0) out vec3 out_texCoord;
layout(location = 1) flat out float out_alpha;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	Flare flare = u_flares[gl_InstanceID];

	out_texCoord = vec3((in_position * 0.5) + 0.5, flare.alphaDepth.y);

	vec4 posScale = flare.posScale;
	gl_Position = vec4(in_position * posScale.zw + posScale.xy , 0.0, 1.0);

	out_alpha = flare.alphaDepth.x;
}
