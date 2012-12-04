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
flat out float vSpecularComponent; ///< Calculate it per fragment
#endif
/// @}
