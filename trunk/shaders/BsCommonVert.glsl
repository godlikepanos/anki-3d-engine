// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Common code for all vertex shaders of BS

#define DEFAULT_FLOAT_PRECISION mediump
#pragma anki include "shaders/MsBsCommon.glsl"

layout(location = POSITION_LOCATION) in vec3 inPosition;
layout(location = SCALE_LOCATION) in float inScale;
layout(location = ALPHA_LOCATION) in float inAlpha;

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
#define particle_DEFINED
void particle(in mat4 mvp)
{
	gl_Position = mvp * vec4(inPosition, 1);
	outAlpha = inAlpha;
	gl_PointSize = inScale * float(RENDERING_WIDTH) / gl_Position.w;
}

//==============================================================================
#define writeAlpha_DEFINED
void writeAlpha(in float alpha)
{
	outAlpha = alpha;
}
