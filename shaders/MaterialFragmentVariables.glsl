/// @name Uniforms
/// @{
#if defined(PASS_COLOR)
#	define blurring_DEFINED
uniform float blurring;
#endif
/// @}

/// @name Varyings
/// @{
#define vTexCoords_DEFINED
in vec2 vTexCoords;

#if defined(PASS_COLOR)
in vec3 vNormal;
#	define vNormal_DEFINED
in vec3 vTangent;
#	define vTangent_DEFINED
in float vTangentW;
#	define vTangentW_DEFINED
in vec3 vVertPosViewSpace;
#	define vVertPosViewSpace_DEFINED
#endif
/// @}

/// @name Fragment out
/// @{
#if defined(PASS_COLOR)
#if 1
layout(location = 0) out uvec2 fMsFai0;
#else
layout(location = 0) out vec3 fMsFai0;
#endif
#	define fMsFai0_DEFINED
#endif
/// @}
