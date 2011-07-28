/// @file
/// Generic vertex shader for material passes (both color and depth)

/// @name Attributes
/// @{
in vec3 position;
in vec3 normal;
in vec2 texCoords;
in vec4 tangent;
/// @}

/// @name Uniforms
/// @{
uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projectionMat;
uniform mat4 modelViewMat;
uniform mat3 normalMat;
uniform mat4 modelViewProjectionMat;
/// @}

/// @name Varyings
/// @{
out vec3 vNormal;
out vec2 vTexCoords;
out vec3 vTangent;
out float vTangentW;
out vec3 vVertPosViewSpace; ///< For env mapping. AKA view vector
/// @}



//==============================================================================
// main                                                                        =
//==============================================================================
void main()
{
	// calculate the vert pos, normal and tangent
	vNormal = normalMat * normal;

	gl_Position = modelViewProjectionMat * vec4(position, 1.0);

	// calculate the rest
	vTexCoords = texCoords;

	vTangent = normalMat * vec3(tangent);
	vTangentW = tangent.w;

	vVertPosViewSpace = vec3(modelViewMat * vec4(position, 1.0));
}
