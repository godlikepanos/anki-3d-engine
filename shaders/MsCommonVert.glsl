#pragma anki include "shaders/MsBsCommon.glsl"
#pragma anki include "shaders/Pack.glsl"

/// @name Attributes
/// @{
layout(location = 0) in vec3 position;
layout(location = 3) in vec2 texCoords;
#if defined(PASS_COLOR)
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 tangent;
#endif
/// @}

/// @name Varyings
/// @{
out vec2 vTexCoords;
#if defined(PASS_COLOR)
out vec3 vNormal;
out vec3 vTangent;
out float vTangentW;
out vec3 vVertPosViewSpace; ///< For env mapping. AKA view vector
/// Calculate it per vertex instead of per fragment
flat out lowp float vSpecularComponent; 
#endif
/// @}

//==============================================================================
#define setVaryings1_DEFINED
void setVaryings1(in mat4 modelViewProjectionMat)
{
#if defined(PASS_DEPTH) && LOD > 0
	// No tex coords for you
#else
	vTexCoords = texCoords;
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
	vNormal = normalMat * normal;
	vTangent = normalMat * vec3(tangent);
	vTangentW = tangent.w;
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

//==============================================================================
#if defined(PASS_COLOR)
#define prepackSpecular_DEFINED
void prepackSpecular(in vec2 specular)
{
	vSpecularComponent = lowp float(packSpecular(specular));
}
#endif

