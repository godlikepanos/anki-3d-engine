/// @file
/// Generic vertex shader for material passes (both color and depth)

#pragma anki include "shaders/Pack.glsl"

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
	vSpecularComponent = packSpecular(specular);
}
#endif
