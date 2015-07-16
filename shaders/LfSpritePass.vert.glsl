// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// LF sprites vert shader

#pragma anki type vert
#pragma anki include "shaders/Common.glsl"

// Per flare information
struct Sprite
{
	vec4 posScale; // xy: Position, z: Scale, w: Scale again
	vec4 alphaDepth; // x: alpha, y: texture depth
};

// The block contains data for all flares
layout(std140) uniform _blk
{
	Sprite u_sprites[MAX_SPRITES];
};

layout(location = 0) out vec3 out_texCoord;
layout(location = 1) flat out float out_alpha;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	const vec2 POSITIONS[4] = vec2[](
		vec2(-1.0, -1.0),
		vec2(1.0, -1.0),
		vec2(-1.0, 1.0),
		vec2(1.0, 1.0));

	vec2 position = POSITIONS[gl_VertexID];

	Sprite sprite = u_sprites[gl_InstanceID];

	// Write tex coords of the 2D array texture
	out_texCoord = vec3((position * 0.5) + 0.5, sprite.alphaDepth.y);

	vec4 posScale = sprite.posScale;
	gl_Position = vec4(position * posScale.zw + posScale.xy , 0.0, 1.0);

	out_alpha = sprite.alphaDepth.x;
}
