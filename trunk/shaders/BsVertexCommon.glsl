layout(location = 0) in vec3 position;
layout(location = 3) in vec2 texCoords;

/// @name Varyings
/// @{
out vec2 vTexCoords;
out flat uint vInstanceId;
/// @}

#pragma anki include "shaders/MaterialCommonFunctions.glsl"

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
	vTexCoords = x;
}

//==============================================================================
#define particle_DEFINED
void particle(in mat4 mvp)
{
	vTexCoords = texCoords;
	gl_Position = mvp * vec4(position, 1);
	vInstanceId = gl_InstanceID;
}
