/// @file
/// Generic vertex shader for material passes (both color and depth)

/// @name Attributes
/// @{
in vec3 position;
in vec2 texCoords;
#if !defined(DEPTH_PASS)
in vec3 normal;
in vec4 tangent;
#endif
/// @}

/// @name Uniforms
/// @{
uniform mat4 modelViewProjectionMat;
#if !defined(DEPTH_PASS)
uniform mat3 normalMat;
uniform mat4 modelViewMat;
#endif
/// @}

/// @name Varyings
/// @{
out vec2 vTexCoords;
#if !defined(DEPTH_PASS)
out vec3 vNormal;
out vec3 vTangent;
out float vTangentW;
out vec3 vVertPosViewSpace; ///< For env mapping. AKA view vector
#endif
/// @}



//==============================================================================
// main                                                                        =
//==============================================================================
void main()
{
#if !defined(DEPTH_PASS)
	// calculate the vert pos, normal and tangent
	vNormal = normalMat * normal;

	vTangent = normalMat * vec3(tangent);
	vTangentW = tangent.w;
	
	vVertPosViewSpace = vec3(modelViewMat * vec4(position, 1.0));
#endif

	vTexCoords = texCoords;
	gl_Position = modelViewProjectionMat * vec4(position, 1.0);
}
