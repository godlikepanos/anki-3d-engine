/// @file
/// Generic vertex shader for material passes (both color and depth)

/// @name Attributes
/// @{
layout(location = 0) in vec3 position;
layout(location = 3) in vec2 texCoords;
#if defined(COLOR_PASS)
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 tangent;
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
out vec2 vTexCoords;
#if defined(COLOR_PASS)
out vec3 vNormal;
out vec3 vTangent;
out float vTangentW;
out vec3 vVertPosViewSpace; ///< For env mapping. AKA view vector
#endif
/// @}


/// Calculate the position and the varyings
void doVertex()
{
#if defined(COLOR_PASS)
	vNormal = normalMat * normal;
	vTangent = normalMat * vec3(tangent);
	vTangentW = tangent.w;
	vVertPosViewSpace = vec3(modelViewMat * vec4(position, 1.0));
#endif

	vTexCoords = texCoords;

	gl_Position = modelViewProjectionMat * vec4(position, 1.0);
}

