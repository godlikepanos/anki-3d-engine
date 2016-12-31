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

layout(location = 0) out vec3 out_vertPosViewSpace;
layout(location = 1) flat out float out_alpha;

out gl_PerVertex
{
	vec4 gl_Position;
	float gl_PointSize;
};

#define setPositionVec3_DEFINED
void setPositionVec3(in vec3 pos)
{
	ANKI_WRITE_POSITION(vec4(pos, 1.0));
}

#define setPositionVec4_DEFINED
void setPositionVec4(in vec4 pos)
{
	ANKI_WRITE_POSITION(pos);
}

#define writePositionMvp_DEFINED
void writePositionMvp(in mat4 mvp)
{
	ANKI_WRITE_POSITION(mvp * vec4(in_position, 1.0));
}

#define particle_DEFINED
void particle(in mat4 mvp)
{
	ANKI_WRITE_POSITION(mvp * vec4(in_position, 1));
	out_alpha = in_alpha;
	gl_PointSize = in_scale * u_lightingUniforms.rendererSizeTimePad1.x * 0.5 / gl_Position.w;
}

#define writeAlpha_DEFINED
void writeAlpha(in float alpha)
{
	out_alpha = alpha;
}

#define writeVertPosViewSpace_DEFINED
void writeVertPosViewSpace(in mat4 modelViewMat)
{
	out_vertPosViewSpace = vec3(modelViewMat * vec4(in_position, 1.0));
}

#endif
