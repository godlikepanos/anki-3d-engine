// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki type vert
#pragma anki include "shaders/IsCommon.glsl"

layout(location = 0) out vec2 out_texCoord;
layout(location = 1) flat out int out_instanceId;
layout(location = 2) out vec2 out_projectionParams;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	float instIdF = float(gl_InstanceID);

	vec2 ij = vec2(
		mod(instIdF, float(TILES_COUNT_X)),
		floor(instIdF / float(TILES_COUNT_X)));

	out_instanceId = int(gl_InstanceID);

	const vec2 SIZES =
		vec2(1.0 / float(TILES_COUNT_X), 1.0 / float(TILES_COUNT_Y));

	const vec2 UVS[4] = vec2[](
		vec2(0.0, 0.0) * SIZES,
		vec2(1.0, 0.0) * SIZES,
		vec2(0.0, 1.0) * SIZES,
		vec2(1.0, 1.0) * SIZES);

	out_texCoord = UVS[gl_VertexID] + ij * SIZES;
	vec2 pos = out_texCoord * 2.0 - 1.0;

	gl_Position = vec4(pos, 0.0, 1.0);
	out_projectionParams = u_projectionParams.xy * pos;
}


