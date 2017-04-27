// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_FS_COMMON_VERT_GLSL
#define ANKI_SHADERS_FS_COMMON_VERT_GLSL

// Common code for all vertex shaders of FS
#include "shaders/MsFsCommon.glsl"

// Global resources
#define LIGHT_SET 1
#define LIGHT_SS_BINDING 0
#define LIGHT_TEX_BINDING 1
#define LIGHT_UBO_BINDING 0
#define LIGHT_MINIMAL
#include "shaders/ClusterLightCommon.glsl"
#undef LIGHT_SET
#undef LIGHT_SS_BINDING
#undef LIGHT_TEX_BINDING

// In/out
layout(location = POSITION_LOCATION) in vec3 in_position;
layout(location = SCALE_LOCATION) in float in_scale;
layout(location = ALPHA_LOCATION) in float in_alpha;

layout(location = 0) flat out float out_alpha;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out vec3 out_posViewSpace;

out gl_PerVertex
{
	vec4 gl_Position;
};

void particle(in mat4 mvp, in mat3 camRot, in mat4 viewMat)
{
	const vec2 POSITIONS[4] = vec2[](vec2(-0.5, -0.5), vec2(0.5, -0.5), vec2(-0.5, 0.5), vec2(0.5, 0.5));
	vec3 worldPos = camRot * vec3(POSITIONS[gl_VertexID] * in_scale, 0.0) + in_position;
	ANKI_WRITE_POSITION(mvp * vec4(worldPos, 1.0));

	out_posViewSpace = (viewMat * vec4(worldPos, 1.0)).xyz;

	out_alpha = in_alpha;

	out_uv = POSITIONS[gl_VertexID] + 0.5;
}

#endif
