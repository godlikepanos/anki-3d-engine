// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// LF sprites vert shader

#include "shaders/Common.glsl"

// Per flare information
struct Sprite
{
	vec4 posScale; // xy: Position, zw: Scale
	vec4 color;
	vec4 depthPad3;
};

// The block contains data for all flares
layout(std140, ANKI_UBO_BINDING(0, 0)) uniform _blk
{
	Sprite u_sprites[MAX_SPRITES];
};

layout(location = 0) out vec3 out_uv;
layout(location = 1) flat out vec4 out_color;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	const vec2 POSITIONS[4] = vec2[](vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, 1.0));

	vec2 position = POSITIONS[gl_VertexID];

	Sprite sprite = u_sprites[gl_InstanceID];

	// Write tex coords of the 2D array texture
	out_uv = vec3((position * 0.5) + 0.5, sprite.depthPad3.x);

	vec4 posScale = sprite.posScale;
	ANKI_WRITE_POSITION(vec4(position * posScale.zw + posScale.xy, 0.0, 1.0));

	out_color = sprite.color;
}
