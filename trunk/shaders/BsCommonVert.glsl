// Common code for all vertex shaders of BS

#define DEFAULT_FLOAT_PRECISION mediump
#pragma anki include "shaders/MsBsCommon.glsl"

layout(location = POSITION_LOCATION) in vec3 position;
layout(location = TEXTURE_COORDINATE_LOCATION) in vec2 texCoord;
layout(location = SCALE_LOCATION) in float scale;
layout(location = ALPHA_LOCATION) in float alpha;

/// @name Varyings
/// @{
out vec2 vTexCoord;
flat out float vAlpha;
/// @}

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
#define setTexCoords_DEFINED
void setTexCoords(in vec2 x)
{
	vTexCoord = x;
}

//==============================================================================
#define particle_DEFINED
void particle(in mat4 mvp)
{
	gl_Position = mvp * vec4(position, 1);
	vAlpha = alpha;
	gl_PointSize = scale * float(RENDERING_WIDTH) / gl_Position.w;
}

//==============================================================================
#define writeAlpha_DEFINED
void writeAlpha(in float alpha)
{
	vAlpha = alpha;
}
