// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Common code for all vertex shaders of BS

#define DEFAULT_FLOAT_PRECISION mediump
#pragma anki include "shaders/MsBsCommon.glsl"

layout(location = POSITION_LOCATION) in vec3 in_position;
layout(location = SCALE_LOCATION) in float in_scale;
layout(location = ALPHA_LOCATION) in float in_alpha;

layout(location = 1) flat out float outAlpha;

out gl_PerVertex
{
	vec4 gl_Position;
	float gl_PointSize;
};

//==============================================================================
#define setPositionVec3_DEFINED
void setPositionVec3(in vec3 pos)
{
	gl_Position = vec4(pos, 1.0);
}

//==============================================================================
#define setPositionVec4_DEFINED
void setPositionVec4(in vec4 pos)
{
	gl_Position = pos;
}

//==============================================================================
#define writePositionMvp_DEFINED
void writePositionMvp(in mat4 mvp)
{
	gl_Position = mvp * vec4(in_position, 1.0);
}

//==============================================================================
#define particle_DEFINED
void particle(in mat4 mvp)
{
	gl_Position = mvp * vec4(in_position, 1);
	outAlpha = in_alpha;
	gl_PointSize = in_scale * float(ANKI_RENDERER_WIDTH) / gl_Position.w;
}

//==============================================================================
#define writeAlpha_DEFINED
void writeAlpha(in float alpha)
{
	outAlpha = alpha;
}
