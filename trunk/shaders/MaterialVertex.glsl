/// @file
/// Generic vertex shader for material passes (both color and depth)

/// @name Attributes
/// @{
layout(location = 0) in vec3 position;
#if defined(USING_TEX_COORDS_ATTRIB)
layout(location = 3) in vec2 texCoords;
#endif
#if defined(COLOR_PASS)
#	if defined(USING_NORMAL_ATTRIB)
layout(location = 1) in vec3 normal;
#	endif
#	if defined(USING_TANGENT_ATTRIB)
layout(location = 2) in vec4 tangent;
#	endif
#endif
/// @}

/// @name Uniforms
/// @{
uniform mat4 modelViewProjectionMat;
uniform mat3 normalMat;
uniform mat4 modelViewMat;
/// @}

/// @name Varyings
/// @{
#if defined(USING_TEX_COORDS_ATTRIB)
out vec2 vTexCoords;
#endif
#if defined(COLOR_PASS)
#	if defined(USING_NORMAL_ATTRIB)
out vec3 vNormal;
#	endif
#	if defined(USING_TANGENT_ATTRIB)
out vec3 vTangent;
out float vTangentW;
#	endif
out vec3 vVertPosViewSpace; ///< For env mapping. AKA view vector
#endif
/// @}



//==============================================================================
// main                                                                        =
//==============================================================================
void main()
{
#if defined(COLOR_PASS)
#	if defined(USING_NORMAL_ATTRIB)
	vNormal = normalMat * normal;
#	endif
#	if defined(USING_TANGENT_ATTRIB)
	vTangent = normalMat * vec3(tangent);
	vTangentW = tangent.w;
#	endif
	vVertPosViewSpace = vec3(modelViewMat * vec4(position, 1.0));
#endif

#if defined(USING_TEX_COORDS_ATTRIB)
	vTexCoords = texCoords;
#endif

	gl_Position = modelViewProjectionMat * vec4(position, 1.0);
}
