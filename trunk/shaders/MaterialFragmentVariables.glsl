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
layout(location = 0) out vec3 fMsNormalFai;
#	define fMsNormalFai_DEFINED
layout(location = 1) out vec3 fMsDiffuseFai;
#	define fMsDiffuseFai_DEFINED
layout(location = 2) out vec4 fMsSpecularFai;
#	define fMsSpecularFai_DEFINED
#endif
/// @}
