/// @name Uniforms
/// @{
#if defined(PASS_COLOR)
uniform float blurring;
#endif
/// @}

/// @name Varyings
/// @{
in vec2 vTexCoords;
#if defined(PASS_COLOR)
in vec3 vNormal;
in vec3 vTangent;
in float vTangentW;
in vec3 vVertPosViewSpace;
#endif
/// @}

/// @name Fragment out
/// @{
#if defined(PASS_COLOR)
layout(location = 0) out vec3 fMsNormalFai;
layout(location = 1) out vec3 fMsDiffuseFai;
layout(location = 2) out vec4 fMsSpecularFai;
#endif
/// @}
