#pragma anki include "shaders/MsBsCommon.glsl"
#pragma anki include "shaders/Pack.glsl"

/// @name Attributes
/// @{
layout(location = 0) in highp vec3 position;
layout(location = 3) in highp vec2 texCoord;
#if defined(PASS_COLOR)
layout(location = 1) in mediump vec3 normal;
layout(location = 2) in mediump vec4 tangent;
#endif
/// @}

/// @name Varyings
/// @{
out highp vec2 vTexCoords;

#ifdef INSTANCING
flat out uint vInstanceId;
#endif

#ifdef PASS_COLOR
out mediump vec3 vNormal;
out mediump vec4 vTangent;
out mediump vec3 vVertPosViewSpace; ///< For env mapping. AKA view vector
#endif
/// @}

//==============================================================================
#define setVaryings1_DEFINED
void setVaryings1(in mat4 modelViewProjectionMat)
{
#if defined(PASS_DEPTH) && LOD > 0
	// No tex coords for you
#else
	vTexCoords = texCoord;
#endif

	gl_Position = modelViewProjectionMat * vec4(position, 1.0);
}

//==============================================================================
#define setVaryings2_DEFINED
void setVaryings2(
	in mat4 modelViewProjectionMat, 
	in mat3 normalMat)
{
#if defined(PASS_COLOR)
	vNormal = vec3(normalMat * normal);
	vTangent.xyz = vec3(normalMat * tangent.xyz);
	vTangent.w = tangent.w;
#endif

	setVaryings1(modelViewProjectionMat);
}

//==============================================================================
#if defined(PASS_COLOR)
#define setVertPosViewSpace_DEFINED
void setVertPosViewSpace(in mat4 modelViewMat)
{
	vVertPosViewSpace = vec3(modelViewMat * vec4(position, 1.0));
}
#endif
